#ifndef MOON_H
#define MOON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Planet.h" // Uključuje osnovne planetarne parametre

class Moon {
private:
    std::vector<float> sphere_vertices;
    std::vector<int> sphere_indices;
    GLuint VBO, VAO, EBO;
    float radius;
    int sectorCount;
    int stackCount;
    float rotationAngle = 0.0f; // Ugao rotacije Meseca oko svoje ose
    float rotationSpeed; // Brzina rotacije Meseca
    float orbitAngle = 0.0f; // Ugao orbite oko planete
    float orbitSpeed; // Brzina orbite
    float distanceFromPlanet; // Udaljenost od planete
    glm::vec3 orbitAxis = glm::vec3(0.0f, 1.0f, 0.0f); // Osa orbite
    Planet& parentPlanet; // Referenca na planetu oko koje orbitira

    void generateVertices();
    void generateIndices();
    void setupMesh();

public:
    Moon(Planet& planet, float r, int sectors, int stacks, float rotSpeed, float orbSpeed, float distance);
    ~Moon();

    void Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection, float deltaTime, float speedMultiplier);
};

#endif // MOON_H
