#define _USE_MATH_DEFINES
#include "SaturnRing.h"

SaturnRing::SaturnRing(int segments, float innerRadius, float outerRadius)
    : segments(segments), innerRadius(innerRadius), outerRadius(outerRadius), VBO(0), VAO(0) {
    generateRingMesh();
}


// Destructor implementation
SaturnRing::~SaturnRing() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

// Function to generate ring mesh
void SaturnRing::generateRingMesh() {
    ring_vertices.clear();
    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * M_PI * i) / segments;
        float x = cos(angle);
        float y = sin(angle);

        // Inner ring
        ring_vertices.push_back(innerRadius * x);
        ring_vertices.push_back(innerRadius * y);
        ring_vertices.push_back(0.0f);
        ring_vertices.push_back((x + 1.0f) * 0.5f);
        ring_vertices.push_back((y + 1.0f) * 0.5f);

        // Outer ring
        ring_vertices.push_back(outerRadius * x);
        ring_vertices.push_back(outerRadius * y);
        ring_vertices.push_back(0.0f);
        ring_vertices.push_back((x + 1.0f) * 0.5f);
        ring_vertices.push_back((y + 1.0f) * 0.5f);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, ring_vertices.size() * sizeof(float), ring_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void SaturnRing::Draw(GLuint shaderProgram, GLuint ringTextureID, glm::mat4 view, glm::mat4 projection, glm::vec3 saturnPosition) {
    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, saturnPosition); // Move the ring with Saturn
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotacija oko X ose
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ringTextureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "ringTexture"), 0);

    glDisable(GL_CULL_FACE);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);

}