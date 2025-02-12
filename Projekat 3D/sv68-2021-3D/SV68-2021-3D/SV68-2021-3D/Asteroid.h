#ifndef ASTEROID_H
#define ASTEROID_H

#include <glad/glad.h>  // Must be first
#include <GLFW/glfw3.h> // Then GLFW
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>



class Asteroid {
public:
    std::vector<float> sphere_vertices;
    std::vector<int> sphere_indices;
    GLuint VBO, VAO, EBO;
    float radius;
    int sectorCount;
    int stackCount;


    void generateVertices();
    void generateIndices();
    void setupMesh();

    float x;
    float y;
    float z;

    Asteroid(float r, int sectors, int stacks, float x = 0.0f, float y = 0.0f, float z = 0.0f);
    ~Asteroid();
};

#endif // ASTEROID_H
