#ifndef SUN_H
#define SUN_H

#include <glad/glad.h>  // Must be first
#include <GLFW/glfw3.h> // Then GLFW
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Sun {
private:
    std::vector<float> sphere_vertices;
    std::vector<int> sphere_indices;
    GLuint VBO, VAO, EBO;
    float radius;
    int sectorCount;
    int stackCount;
    float rotationAngle = 0.0f; // Uglovi rotacije u stepenima
    float rotationSpeed = 10.0f; // Stepeni po sekundi

    void generateVertices();
    void generateIndices();
    void setupMesh();

public:
    Sun(float r, int sectors, int stacks);
    ~Sun();

    glm::vec3 getPosition() const;
    float getRadius() const;
    void Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, float deltaTime, glm::vec3 cameraPos);
};

#endif // SUN_H
