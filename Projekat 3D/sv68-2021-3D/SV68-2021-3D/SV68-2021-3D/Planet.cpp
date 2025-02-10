#define _USE_MATH_DEFINES
#include <cmath>
#include "Planet.h"


Planet::Planet(float r, int sectors, int stacks, float rotSpeed, float orbSpeed, float distance, float ecc)
    : radius(r), sectorCount(sectors), stackCount(stacks),
    rotationSpeed(rotSpeed), orbitSpeed(orbSpeed), distanceFromSun(distance), eccentricity(ecc) {

    // Generate sphere vertices and indices
    generateVertices();
    generateIndices();

    // Setup OpenGL buffers and attributes
    setupMesh();

    generateOrbit();
    setupOrbitMesh();
}


Planet::~Planet() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Planet::generateVertices() {
    float x, y, z, xy;
    float s, t;
    float sectorStep = (float)(2 * M_PI / sectorCount);
    float stackStep = (float)(M_PI / stackCount);
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = (float)(M_PI / 2 - i * stackStep);
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            y = xy * cosf(sectorAngle); // Ranije x, sada y
            x = xy * sinf(sectorAngle); // Ranije y, sada x

            // UV koordinate
            s = (float)j / (float)(sectorCount);
            t = 1.0f - (float)i / (float)(stackCount);


            if (i == 0) t = 0.01f; // Avoid collapsing at the north pole
            if (i == stackCount) t = 0.99f; // Avoid collapsing at the south pole

            // Dodavanje verteksa
            sphere_vertices.push_back(x);
            sphere_vertices.push_back(y);
            sphere_vertices.push_back(z);
            sphere_vertices.push_back(s);
            sphere_vertices.push_back(t);
        }
    }
}

void Planet::generateIndices() {
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                sphere_indices.push_back(k1);
                sphere_indices.push_back(k2);
                sphere_indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                sphere_indices.push_back(k1 + 1);
                sphere_indices.push_back(k2);
                sphere_indices.push_back(k2 + 1);
            }
        }
    }

    // Zatvaranje donjeg pola
    int bottomCenterIndex = (int)sphere_vertices.size() / 5 - 1;
    int lastRowStart = bottomCenterIndex - sectorCount;
    for (int j = 0; j < sectorCount; ++j) {
        int next = (j == sectorCount - 1) ? lastRowStart : lastRowStart + j + 1;
        sphere_indices.push_back(bottomCenterIndex);
        sphere_indices.push_back(lastRowStart + j);
        sphere_indices.push_back(next);
    }
}

void Planet::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size() * sizeof(float), sphere_vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_indices.size() * sizeof(int), sphere_indices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void Planet::Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, float deltaTime, glm::vec3 cameraPos, float speedMultiplier) {
    // Ažuriranje ugla orbite i rotacije planete
    orbitAngle += orbitSpeed * deltaTime * speedMultiplier;
    if (orbitAngle > 360.0f) orbitAngle -= 360.0f;

    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

    // Dobijanje pravilne pozicije planete u orbiti
    glm::vec3 position = getPosition();

    // Kreiranje model matrice
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position); // Postavi planetu u orbitu

    // **Prvo rotacija oko Y ose za normalnu rotaciju planete**
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // **Zatim ispravi početnu orijentaciju (ako je potrebno)**
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Na kraju skaliraj model
    model = glm::scale(model, glm::vec3(radius));

    // Koristi šejder
    glUseProgram(shaderProgram);

    // Prosleđivanje uniform vrednosti u šejder
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));

    // Bindovanje teksture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "planetTexture"), 0);

    // Iscrtavanje planete
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, sphere_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}



glm::vec3 Planet::getPosition() {
    float orbitRadians = glm::radians(orbitAngle);

    // Pravilna eliptična orbita (poluvelika i polumana osa)
    float semiMajorAxis = distanceFromSun;  // Poluvelika osa (a)
    float semiMinorAxis = distanceFromSun * sqrt(1 - eccentricity * eccentricity); // Polumana osa (b)

    // Izračunavanje eliptične pozicije
    float x = cos(orbitRadians) * semiMajorAxis - semiMajorAxis * eccentricity; // Translacija da Sunce bude u žarištu
    float z = sin(orbitRadians) * semiMinorAxis;

    return glm::vec3(x, 0.0f, z);
}


float Planet::getRadius() const {
    return radius;
}

void Planet::generateOrbit() {
    int numSegments = 100; // Broj tačaka za crtanje glatke elipse
    float angleStep = 2.0f * M_PI / numSegments;

    // Poluvelika i polumana osa elipse
    float semiMajorAxis = distanceFromSun;  // Poluvelika osa (a)
    float semiMinorAxis = distanceFromSun * sqrt(1 - eccentricity * eccentricity); // Polumana osa (b)

    for (int i = 0; i < numSegments; i++) {
        float angle = i * angleStep;
        float x = cos(angle) * semiMajorAxis - semiMajorAxis * eccentricity; // Translacija da Sunce bude u žarištu
        float z = sin(angle) * semiMinorAxis;

        orbit_vertices.push_back(glm::vec3(x, 0.0f, z));
    }
}


void Planet::setupOrbitMesh() {
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, orbit_vertices.size() * sizeof(glm::vec3), orbit_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void Planet::DrawOrbit(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);

    // Kreiranje model matrice (osigurava da su orbite u istom prostoru kao planete)
    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Postavi boju orbite (uniform promenljiva u sejderu)
    glm::vec3 orbitColor = glm::vec3(0.8f, 0.8f, 0.8f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "orbitColor"), 1, glm::value_ptr(orbitColor));

    glBindVertexArray(orbitVAO);
    glDrawArrays(GL_LINE_LOOP, 0, orbit_vertices.size()); // Iscrtavanje orbite
    glBindVertexArray(0);
}

