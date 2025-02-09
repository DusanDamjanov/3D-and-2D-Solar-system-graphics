#ifndef SATURNRING_H
#define SATURNRING_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

class SaturnRing {
private:
    std::vector<float> ring_vertices;
    GLuint VBO, VAO;
    int segments;
    float innerRadius;
    float outerRadius;
    glm::vec3 saturnPosition;  

public:
    SaturnRing(int segments, float innerRadius, float outerRadius);
    ~SaturnRing();

    void generateRingMesh();
    void Draw(GLuint shaderProgram, GLuint ringTextureID, glm::mat4 view, glm::mat4 projection, glm::vec3 saturnPosition);
};

#endif // SATURNRING_H
