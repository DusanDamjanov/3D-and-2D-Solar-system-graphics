#ifndef SUN_H
#define SUN_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Sun3D {
public:
    glm::vec3 position;
    float size;
    float rotationSpeed;
    float currentAngle;
    GLuint shaderProgram;
    GLuint VAO, VBO, EBO;
    GLuint textureID;

    Sun3D(glm::vec3 pos, float sz, float rotSpeed, GLuint program, const char* texturePath);
    void update(float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    void generateSphereData();
};

#endif // SUN_H
