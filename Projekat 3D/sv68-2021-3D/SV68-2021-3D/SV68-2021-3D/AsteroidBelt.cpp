#define _USE_MATH_DEFINES
#include <cmath>
#include "AsteroidBelt.h"

AsteroidBelt::AsteroidBelt(int count, float inner, float outer)
    : numAsteroids(count), innerRadius(inner), outerRadius(outer), baseAsteroid(0.3f, 8, 8) {
    generateAsteroids();
    setupInstancedRendering();
}


AsteroidBelt::~AsteroidBelt() {
    glDeleteBuffers(1, &instanceVBO);
    modelMatrices.clear();
}


void AsteroidBelt::generateAsteroids() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(innerRadius, outerRadius);
    std::uniform_real_distribution<float> heightDist(-1.0f, 1.0f);

    modelMatrices.clear();

    for (int i = 0; i < numAsteroids; ++i) {
        float angle = angleDist(gen);
        float radius = radiusDist(gen);
        float height = heightDist(gen);

        float x = radius * cos(angle);
        float y = height;
        float z = radius * sin(angle);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));
        model = glm::scale(model, glm::vec3(0.05f));

        modelMatrices.push_back(model);
    }

}


void AsteroidBelt::setupInstancedRendering() {
    glBindVertexArray(baseAsteroid.VAO);
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    glBufferData(GL_ARRAY_BUFFER, numAsteroids * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(2 + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void AsteroidBelt::Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, glm::vec3 cameraPos) {
    glUseProgram(shaderProgram);

    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1f(glGetUniformLocation(shaderProgram, "glowIntensity"), 0.3f); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "asteroidTexture"), 0);

    glBindVertexArray(baseAsteroid.VAO);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(baseAsteroid.sphere_indices.size()), GL_UNSIGNED_INT, 0, numAsteroids);
    glBindVertexArray(0);
}

bool AsteroidBelt::isInsideBelt(glm::vec3 cameraPos) {
    float distance = glm::length(cameraPos);    //razdaljina od (0, 0, 0)

    if (distance >= innerRadius && distance <= outerRadius && -1.0f <= cameraPos.y && cameraPos.y <= 1.0f) {
        return true;
    }
    return false;
}

