#define _USE_MATH_DEFINES


#include <cmath>
#include "Sun.h"
#include "SV68-2021-3D.h"  
#include <iostream>
#include <vector>
#include "stb_image.h"


Sun3D::Sun3D(glm::vec3 pos, float sz, float rotSpeed, GLuint program, const char* texturePath)
    : position(pos), size(sz), rotationSpeed(rotSpeed), shaderProgram(program), currentAngle(0.0f) {
    textureID = loadTexture(texturePath);
    generateSphereData();
}

void Sun3D::update(float deltaTime) {
    currentAngle += rotationSpeed * deltaTime;
    if (currentAngle > 360.0f) currentAngle -= 360.0f;
}

void Sun3D::generateSphereData() {
    const int sectors = 36; // Broj horizontalnih delova sfere
    const int stacks = 18;  // Broj vertikalnih delova sfere

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks; // Vertikalna koordinata (od pola do pola)

        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * M_PI * j / sectors; // Horizontalna koordinata (od 0 do 2π)

            // Pozicije verteksa
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            // UV koordinate
            float u = (float)j / (float)sectors;
            float v = (float)i / (float)stacks;

            // Debug ispis UV koordinata
            std::cout << "UV[" << i << "," << j << "]: u=" << u << ", v=" << v << std::endl;

            // Provera da li su UV vrednosti u validnom opsegu [0,1]
            if (u < 0.0f || v < 0.0f || u > 1.0f || v > 1.0f) {
                std::cerr << "GRESKA! UV izvan opsega: u=" << u << ", v=" << v << std::endl;
            }


            vertices.push_back(x * size + position.x); // x pozicija
            vertices.push_back(y * size + position.y); // y pozicija
            vertices.push_back(z * size + position.z); // z pozicija
            vertices.push_back(x); // Normal x
            vertices.push_back(y); // Normal y
            vertices.push_back(z); // Normal z
            vertices.push_back(u); // UV u
            vertices.push_back(v); // UV v
        }
    }

    // Generisanje indeksa za crtanje elemenata
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int first = i * (sectors + 1) + j;
            int second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    std::cout << "Generisano verteksa: " << vertices.size() / 8 << std::endl;
    std::cout << "Generisano indeksa: " << indices.size() << std::endl;

    // OpenGL generisanje
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}




void Sun3D::draw(const glm::mat4& view, const glm::mat4& projection) {

    glUseProgram(shaderProgram);
    checkOpenGLError("glUseProgram");

    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        checkOpenGLError("glActiveTexture");

        glBindTexture(GL_TEXTURE_2D, textureID);
        checkOpenGLError("glBindTexture");

        glUniform1i(glGetUniformLocation(shaderProgram, "sunTexture"), 0);
        checkOpenGLError("glUniform1i (sunTexture)");
    }
    else {
        std::cerr << "Error: Sun texture not loaded!" << std::endl;
    }

    glUniform3f(glGetUniformLocation(shaderProgram, "sunColor"), 1.0f, 0.8f, 0.0f);
    checkOpenGLError("glUniform3f (sunColor)");

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    if (modelLoc == -1 || viewLoc == -1 || projLoc == -1) {
        std::cerr << "Error: One or more uniform locations are invalid!" << std::endl;
        return;
    }

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(currentAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    /*    std::cout << "Model Matrix:\n" << glm::to_string(model) << std::endl;
        std::cout << "View Matrix:\n" << glm::to_string(view) << std::endl;
        std::cout << "Projection Matrix:\n" << glm::to_string(projection) << std::endl;*/

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    checkOpenGLError("glUniformMatrix4fv (model)");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    checkOpenGLError("glUniformMatrix4fv (view)");

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    checkOpenGLError("glUniformMatrix4fv (projection)");

    glBindVertexArray(VAO);
    checkOpenGLError("glBindVertexArray");

    glDrawElements(GL_TRIANGLE_STRIP, 36 * 18 * 6, GL_UNSIGNED_INT, 0);
    checkOpenGLError("glDrawArrays");

    glBindVertexArray(0);
}