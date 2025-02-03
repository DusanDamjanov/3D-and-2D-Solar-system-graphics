#pragma once
#ifndef SV68_H
#define SV68_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>

// **Standard Library Includes**
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// **Include GLM for Matrix Operations**
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// **Include STB Image for Textures**
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// **Other Headers
#include "Sun.h"

// Deklaracija funkcije za učitavanje teksture
GLuint loadTexture(const char* filePath);

void checkOpenGLError(const std::string& location);

#endif