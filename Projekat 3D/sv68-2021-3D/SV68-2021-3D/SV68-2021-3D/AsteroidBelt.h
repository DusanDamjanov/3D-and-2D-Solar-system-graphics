#ifndef ASTEROID_BELT_H
#define ASTEROID_BELT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Asteroid.h"

class AsteroidBelt {
public:
    Asteroid baseAsteroid;
    std::vector<glm::mat4> modelMatrices;
    int numAsteroids;
    float innerRadius, outerRadius;
    GLuint instanceVBO;

    AsteroidBelt(int count, float inner, float outer);
    ~AsteroidBelt();

    void generateAsteroids();
    void setupInstancedRendering();
    bool isInsideBelt(glm::vec3 cameraPos);
    void Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, glm::vec3 cameraPos);
};

#endif // ASTEROID_BELT_H