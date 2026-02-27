#include "core/resource_manager.h"

#include <filesystem>
#include <utility>

// Instantiate static variables
std::map<std::string, std::shared_ptr<Shader>> ResourceManager::Shaders;
std::map<std::string, std::shared_ptr<Texture>> ResourceManager::Textures;
std::map<std::string, std::shared_ptr<Mesh>> ResourceManager::Meshes;

std::shared_ptr<Shader> ResourceManager::LoadShader(std::string name, const char* vShaderFile,
                                                    const char* fShaderFile) {
  Shaders.emplace(name, std::make_shared<Shader>(vShaderFile, fShaderFile));
  return Shaders.at(name);
}

std::shared_ptr<Shader> ResourceManager::GetShader(std::string name) { return Shaders.at(name); }

std::vector<std::string> ResourceManager::GetShaderNames() {
  std::vector<std::string> names;
  for (auto& shader : Shaders) {
    names.push_back(shader.first);
  }
  return names;
}

std::shared_ptr<Texture> ResourceManager::LoadTexture(std::string name, const char* file) {
  Textures.emplace(name, std::make_shared<Texture>(file));
  return Textures.at(name);
}

std::shared_ptr<Texture> ResourceManager::GetTexture(std::string name) { return Textures.at(name); }

std::vector<std::string> ResourceManager::GetTextureNames() {
  std::vector<std::string> names;
  for (auto& texture : Textures) {
    names.push_back(texture.first);
  }
  return names;
}

std::shared_ptr<Mesh> ResourceManager::LoadMesh(const char* file) {
  auto name = std::filesystem::path(file).stem().c_str();
  return LoadMesh(name, file);
}

std::shared_ptr<Mesh> ResourceManager::LoadMesh(std::string name, const char* file) {
  Meshes.emplace(name, std::make_shared<Mesh>(file));
  return Meshes.at(name);
}

std::shared_ptr<Mesh> ResourceManager::GetMesh(std::string name) { return Meshes.at(name); }

void ResourceManager::Clear() {
  Shaders.clear();
  Textures.clear();
  Meshes.clear();
}