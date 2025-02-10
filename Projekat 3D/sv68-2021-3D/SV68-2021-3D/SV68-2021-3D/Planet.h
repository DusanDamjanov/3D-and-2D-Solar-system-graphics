#ifndef PLANET_H
#define PLANET_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Planet {
private:
    std::vector<float> sphere_vertices;
    std::vector<int> sphere_indices;
    GLuint VBO, VAO, EBO;
    float radius;
    int sectorCount;
    int stackCount;
    float rotationAngle = 0.0f; // Ugao rotacije planete oko sebe
    float rotationSpeed; // Brzina rotacije oko svoje ose
    float orbitAngle = 0.0f; // Trenutni ugao planete u orbiti
    float orbitSpeed; // Brzina orbite
    float distanceFromSun; // Udaljenost od Sunca
    glm::vec3 orbitAxis = glm::vec3(0.0f, 1.0f, 0.0f); // Osa orbite (oko Y ose)

    void generateVertices();
    void generateIndices();
    void setupMesh();

    std::vector<glm::vec3> orbit_vertices; // Tačke za orbitu
    GLuint orbitVAO, orbitVBO; // OpenGL resursi za orbitu
    float eccentricity;

    void generateOrbit(); // Generisanje tačaka orbite
    void setupOrbitMesh(); // Postavljanje OpenGL bafera

public:
    Planet(float r, int sectors, int stacks, float rotSpeed, float orbSpeed, float distance, float ecc);
    ~Planet();

    void DrawOrbit(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection); // Crtanje orbite


    glm::vec3 getPosition();

    float getRadius() const;

    void Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, float deltaTime, glm::vec3 cameraPos, float speedMultiplier);
};

#endif // PLANET_H
