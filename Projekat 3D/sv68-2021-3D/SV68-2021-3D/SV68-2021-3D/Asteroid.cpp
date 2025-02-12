#define _USE_MATH_DEFINES
#include <cmath>
#include "Asteroid.h"

Asteroid::Asteroid(float r, int sectors, int stacks, float x, float y, float z)
    : radius(r), sectorCount(sectors), stackCount(stacks), x(x), y(y), z(z) {
    generateVertices();
    generateIndices();
    setupMesh();
}

Asteroid::~Asteroid() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Asteroid::generateVertices() {
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
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // UV koordinate
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

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

void Asteroid::generateIndices() {
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

void Asteroid::setupMesh() {
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
