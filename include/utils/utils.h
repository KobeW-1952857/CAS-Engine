#pragma once

#include <glm/glm.hpp>
#include <string>

using Point = glm::vec3;
using Direction = glm::vec3;
using TextureCoord = glm::vec2;
using Color = glm::vec4;

std::string readFile(const char* filePath);
void checkOpenGLError(const std::string& label);