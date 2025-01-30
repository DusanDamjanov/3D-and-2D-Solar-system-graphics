#pragma once
#ifndef SV68_H
#define SV68_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>


// Deklaracija funkcije za učitavanje teksture
GLuint loadTexture(const char* filePath);

void checkOpenGLError(const std::string& location);

#endif