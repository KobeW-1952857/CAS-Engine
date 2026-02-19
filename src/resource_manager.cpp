#include "resource_manager.h"

#include <utility>

// Instantiate static variables
std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture> ResourceManager::Textures;

Shader& ResourceManager::LoadShader(std::string name, const char* vShaderFile, const char* fShaderFile) {
  Shaders.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                  std::forward_as_tuple(vShaderFile, fShaderFile));
  return Shaders.at(name);
}

Shader& ResourceManager::GetShader(std::string name) { return Shaders.at(name); }

Texture& ResourceManager::LoadTexture(std::string name, const char* file) {
  Textures.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(file));
  return Textures.at(name);
}

Texture& ResourceManager::GetTexture(std::string name) { return Textures.at(name); }

void ResourceManager::Clear() {
  Shaders.clear();
  Textures.clear();
}