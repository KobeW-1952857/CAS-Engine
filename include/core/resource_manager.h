#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

class ResourceManager {
 public:
  static std::shared_ptr<Shader> LoadShader(std::string name, const char* vShaderFile, const char* fShaderFile);
  static std::shared_ptr<Shader> GetShader(std::string name);
  static std::vector<std::string> GetShaderNames();

  static std::shared_ptr<Texture> LoadTexture(std::string name, const char* file);
  static std::shared_ptr<Texture> GetTexture(std::string name);
  static std::vector<std::string> GetTextureNames();

  static std::shared_ptr<Material> LoadMaterial(std::string name);
  static std::shared_ptr<Material> LoadMaterial(std::string name, std::shared_ptr<Shader> shader,
                                                std::shared_ptr<Texture> texture);
  static std::shared_ptr<Material> GetMaterial(std::string name);
  static std::vector<std::string> GetMaterialNames();

  static std::shared_ptr<Mesh> LoadMesh(const char* file);
  static std::shared_ptr<Mesh> LoadMesh(std::string name, const char* file);
  static std::shared_ptr<Mesh> GetMesh(std::string name);

  static void Clear();

 private:
  ResourceManager() {}

  static std::map<std::string, std::shared_ptr<Shader>> Shaders;
  static std::map<std::string, std::shared_ptr<Texture>> Textures;
  static std::map<std::string, std::shared_ptr<Mesh>> Meshes;
};