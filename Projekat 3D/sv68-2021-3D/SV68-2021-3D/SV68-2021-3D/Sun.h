//#ifndef SUN_H
//#define SUN_H
//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <string>
//
//class Sun3D {
//public:
//    GLuint VAO, VBO, EBO;
//    GLuint textureID;
//    int indexCount;
//    float currentAngle;
//    float rotationSpeed;
//    glm::vec3 position;
//    float size;
//    GLuint shaderProgram;
//
//    Sun3D(glm::vec3 pos, float sz, float rotSpeed, GLuint program, const char* texturePath);
//    void update(float deltaTime);
//    void draw(const glm::mat4& view, const glm::mat4& projection);
//
//private:
//    void generateSphereData();
//};
//
//#endif // SUN_H


#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>  // Must be first
#include <GLFW/glfw3.h> // Then GLFW
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Sphere {
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
    Sphere(float r, int sectors, int stacks);
    ~Sphere();

    void Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, float deltaTime, glm::vec3 cameraPos);
};

#endif // SPHERE_H
