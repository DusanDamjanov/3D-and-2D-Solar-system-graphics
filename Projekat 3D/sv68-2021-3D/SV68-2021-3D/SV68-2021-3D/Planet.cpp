#define _USE_MATH_DEFINES
#include <cmath>
#include "Planet.h"


Planet::Planet(float r, int sectors, int stacks, float rotSpeed, float orbSpeed, float distance)
    : radius(r), sectorCount(sectors), stackCount(stacks),
    rotationSpeed(rotSpeed), orbitSpeed(orbSpeed), distanceFromSun(distance) {

    // Generate sphere vertices and indices
    generateVertices();
    generateIndices();

    // Setup OpenGL buffers and attributes
    setupMesh();
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


void Planet::Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, float deltaTime, glm::vec3 cameraPos) {
    // Update rotation and orbit angles
    orbitAngle += orbitSpeed * deltaTime;
    if (orbitAngle > 360.0f) orbitAngle -= 360.0f;

    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

    // Compute the planet's position in its orbit
    float orbitRadians = glm::radians(orbitAngle);
    glm::vec3 position = glm::vec3(
        cos(orbitRadians) * distanceFromSun,
        0.0f,
        sin(orbitRadians) * distanceFromSun
    );

    // Compute model transformation matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position); // Postavi planetu u orbitu

    // **Prvo rotacija oko Y ose za normalnu rotaciju planete**
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // **Zatim ispravi početnu orijentaciju (ako je potrebno)**
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Na kraju skaliraj model
    model = glm::scale(model, glm::vec3(radius));


    // Use shader program
    glUseProgram(shaderProgram);

    // Send uniforms to shader
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "planetTexture"), 0);

    // Bind VAO and draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, sphere_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
