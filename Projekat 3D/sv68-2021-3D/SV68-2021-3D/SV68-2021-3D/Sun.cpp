//#include "Sun.h"
//#include "SV68-2021-3D.h"  
//#include <iostream>
//#include <vector>
//
//
//Sun3D::Sun3D(glm::vec3 pos, float sz, float rotSpeed, GLuint program, const char* texturePath)
//    : position(pos), size(sz), rotationSpeed(rotSpeed), shaderProgram(program), currentAngle(0.0f) {
//    textureID = loadTexture(texturePath);
//    generateSphereData();
//}
//
//void Sun3D::update(float deltaTime) {
//    currentAngle += rotationSpeed * deltaTime;
//    if (currentAngle > 360.0f) currentAngle -= 360.0f;
//}
//
//void Sun3D::generateSphereData() {
//	std::vector<float> sphere_vertices;
//	std::vector<float> sphere_texcoord;
//	std::vector<int> sphere_indices;
//
//	int sectorCount = 36;
//	int stackCount = 18;
//
//
//	/* GENERATE VERTEX ARRAY */
//	float x, y, z, xy;                              // vertex position
//	float lengthInv = 1.0f / size;    // vertex normal
//	float s, t;                                     // vertex texCoord
//
//	float sectorStep = (float)(2 * M_PI / sectorCount);
//	float stackStep = (float)(M_PI / stackCount);
//	float sectorAngle, stackAngle;
//
//	for (int i = 0; i <= stackCount; ++i)
//	{
//		stackAngle = (float)(M_PI / 2 - i * stackStep);        // starting from pi/2 to -pi/2
//		xy = 1.02f * size * cosf(stackAngle);             // r * cos(u)
//		z = size * sinf(stackAngle);              // r * sin(u)
//
//		// add (sectorCount+1) vertices per stack
//		// the first and last vertices have same position and normal, but different tex coords
//		for (int j = 0; j <= sectorCount; ++j)
//		{
//			sectorAngle = j * sectorStep;           // starting from 0 to 2pi
//
//			// vertex position (x, y, z)
//			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
//			y = xy * sinf(sectorAngle);				// r * cos(u) * sin(v)
//			sphere_vertices.push_back(x);
//			sphere_vertices.push_back(y);
//			sphere_vertices.push_back(z);
//
//
//			// vertex tex coord (s, t) range between [0, 1]
//			s = (float)j / sectorCount;
//			t = (float)i / stackCount;
//			sphere_vertices.push_back(s);
//			sphere_vertices.push_back(t);
//
//		}
//	}
//	/* GENERATE VERTEX ARRAY */
//
//
//	/* GENERATE INDEX ARRAY */
//	int k1, k2;
//	for (int i = 0; i < stackCount; ++i)
//	{
//		k1 = i * (sectorCount + 1);     // beginning of current stack
//		k2 = k1 + sectorCount + 1;      // beginning of next stack
//
//		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
//		{
//			// 2 triangles per sector excluding first and last stacks
//			// k1 => k2 => k1+1
//			if (i != 0)
//			{
//				sphere_indices.push_back(k1);
//				sphere_indices.push_back(k2);
//				sphere_indices.push_back(k1 + 1);
//			}
//
//			// k1+1 => k2 => k2+1
//			if (i != (stackCount - 1))
//			{
//				sphere_indices.push_back(k1 + 1);
//				sphere_indices.push_back(k2);
//				sphere_indices.push_back(k2 + 1);
//			}
//		}
//	}
//	/* GENERATE INDEX ARRAY */
//
//
//	/* GENERATE VAO-EBO */
//	//GLuint VBO, VAO, EBO;
//	glGenVertexArrays(1, &VAO);
//	glGenBuffers(1, &VBO);
//	glGenBuffers(1, &EBO);
//	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
//	glBindVertexArray(VAO);
//
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, (unsigned int)sphere_vertices.size() * sizeof(float), sphere_vertices.data(), GL_DYNAMIC_DRAW);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)sphere_indices.size() * sizeof(unsigned int), sphere_indices.data(), GL_DYNAMIC_DRAW);
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//	/* GENERATE VAO-EBO */
//
//
//	glBindVertexArray(VAO);
//	glDrawElements(GL_TRIANGLES,
//		(unsigned int)sphere_indices.size(),
//		GL_UNSIGNED_INT,
//		(void*)0);
//	glBindVertexArray(0);
//
//}
//
//
//
//
//void Sun3D::draw(const glm::mat4& view, const glm::mat4& projection) {
//
//    glUseProgram(shaderProgram);
//
//    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
//    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//
//    glUniform3f(glGetUniformLocation(shaderProgram, "sunColor"), 1.0f, 0.8f, 0.0f);
//
//    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
//    model = glm::rotate(model, glm::radians(currentAngle), glm::vec3(0.0f, 1.0f, 0.0f));
//    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
//
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glUniform1i(glGetUniformLocation(shaderProgram, "sunTexture"), 0);
//
//    glBindVertexArray(VAO);
//    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//}




#define _USE_MATH_DEFINES
#include <cmath>
#include "Sun.h"

Sphere::Sphere(float r, int sectors, int stacks)
    : radius(r), sectorCount(sectors), stackCount(stacks) {
    generateVertices();
    generateIndices();
    setupMesh();
}

Sphere::~Sphere() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Sphere::generateVertices() {
    float x, y, z, xy;
    float s, t;
    float sectorStep = (float)(2 * M_PI / sectorCount);
    float stackStep = (float)(M_PI / stackCount);
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = (float)(M_PI / 2 - i * stackStep);
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // UV koordinate
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            if (i == 0) t = 0.01f; // Avoid collapsing at the north pole
            if (i == stackCount) t = 0.99f; // Avoid collapsing at the south pole

            // Dodavanje verteksa
            sphere_vertices.push_back(x);
            sphere_vertices.push_back(y);
            sphere_vertices.push_back(z);
            sphere_vertices.push_back(s);
            sphere_vertices.push_back(t);
        }
    }
}

void Sphere::generateIndices() {
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                sphere_indices.push_back(k1);
                sphere_indices.push_back(k2);
                sphere_indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                sphere_indices.push_back(k1 + 1);
                sphere_indices.push_back(k2);
                sphere_indices.push_back(k2 + 1);
            }
        }
    }

    // Zatvaranje donjeg pola
    int bottomCenterIndex = (int)sphere_vertices.size() / 5 - 1;
    int lastRowStart = bottomCenterIndex - sectorCount;
    for (int j = 0; j < sectorCount; ++j) {
        int next = (j == sectorCount - 1) ? lastRowStart : lastRowStart + j + 1;
        sphere_indices.push_back(bottomCenterIndex);
        sphere_indices.push_back(lastRowStart + j);
        sphere_indices.push_back(next);
    }
}

void Sphere::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size() * sizeof(float), sphere_vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_indices.size() * sizeof(int), sphere_indices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//void Sphere::Draw(GLuint shaderProgram, GLuint textureID) {
//
//    glUseProgram(shaderProgram);
//
//    //glActiveTexture(GL_TEXTURE0);
//    //glBindTexture(GL_TEXTURE_2D, textureID);
//    //// Find the location of the uniform in the shader
//    //GLint colorLocation = glGetUniformLocation(shaderProgram, "objectColor");
//
//    //// Set the color (R, G, B) values
//    //glUniform3f(colorLocation, 1.0f, 0.5f, 0.2f);  // Example: Orange color
//
//
//    glm::mat4 modelMatrix = glm::mat4(1.0f);
//    modelMatrix = glm::translate(modelMatrix, position);  // Pozicija Sunca
//    modelMatrix = glm::rotate(modelMatrix, glm::radians(currentAngle), glm::vec3(0.0f, 1.0f, 0.0f));
//
//    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
//    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
//    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
//
//    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
//    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glUniform1i(glGetUniformLocation(shaderProgram, "sunTexture"), 0);
//
//
//
//    glBindVertexArray(VAO);
//    glDrawElements(GL_TRIANGLES, sphere_indices.size(), GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//
//
//}



void Sphere::Draw(GLuint shaderProgram, GLuint textureID, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);

    // Model matrix setup
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));  // Sphere at origin
    modelMatrix = glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // No rotation for now

    // Locate uniform variables in the shader
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Pass matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "sphereTexture"), 0);

    // Draw the sphere
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphere_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
}
