
#ifndef SKYBOX_H
#define SKYBOX_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SkyBox {
private:
    std::vector<float> skyboxVertices;        //definisanje vertexa (kocka)
    GLuint skyboxVAO, skyboxVBO;
    std::vector<std::string> pictures;          //imena fajlova
    GLuint cubemapTexture;
    GLuint skyboxProgram;
public:
    GLuint textureID;
    void initializeSkybox();
    void renderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    SkyBox(GLuint skyboxProgram, GLuint textureID);
    ~SkyBox();
};

#endif // SKYBOX_H








