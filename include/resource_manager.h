#pragma once

#include <map>
#include <string>

#include "shader.h"
#include "texture.h"

class ResourceManager {
 public:
  static Shader& LoadShader(std::string name, const char* vShaderFile, const char* fShaderFile);
  static Shader& GetShader(std::string name);

  static Texture& LoadTexture(std::string name, const char* file);
  static Texture& GetTexture(std::string name);

  static void Clear();

 private:
  ResourceManager() {}

  static std::map<std::string, Shader> Shaders;
  static std::map<std::string, Texture> Textures;
};