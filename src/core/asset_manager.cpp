#include "core/asset_manager.h"

#include <memory>

void AssetManager::init() {
  auto default_shader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
  default_shader->handle = ++s_ENGINE_ASSETS;
  addMemoryOnlyAsset(default_shader, "default");

  auto default_material = std::make_shared<Material>(default_shader);
  default_material->handle = ++s_ENGINE_ASSETS;
  addMemoryOnlyAsset(default_material, "default");
  default_material->setProperty("u_color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

  auto cube_mesh = std::make_shared<Mesh>("assets/models/cube.obj");
  cube_mesh->handle = ++s_ENGINE_ASSETS;
  addMemoryOnlyAsset(cube_mesh, "cube");

  auto sphere_mesh = std::make_shared<Mesh>("assets/models/sphere.obj");
  sphere_mesh->handle = ++s_ENGINE_ASSETS;
  addMemoryOnlyAsset(sphere_mesh, "sphere");
}

void AssetManager::serialize(YAML::Emitter& out) {
  out << YAML::Key << "AssetRegistry";
  out << YAML::Value << YAML::BeginSeq;

  for (const auto& [id, meta] : s_asset_registry) {
    out << YAML::BeginMap;
    out << YAML::Key << "ID" << YAML::Value << (uint64_t)id;
    out << YAML::Key << "Type" << YAML::Value << (int)meta.type;
    out << YAML::Key << "Filepath" << YAML::Value << meta.filepath;
    out << YAML::EndMap;
  }

  out << YAML::EndSeq;
}

void AssetManager::deserialize(const YAML::Node& node) {
  auto registry_node = node["AssetRegistry"];
  if (!registry_node) return;
  s_asset_registry.clear();
  s_loaded_assets.clear();

  for (const auto& asset : registry_node) {
    AssetMetadata metadata{
        .handle = asset["ID"].as<uint64_t>(),
        .type = (AssetType)asset["Type"].as<int>(),
        .filepath = asset["Filepath"].as<std::string>(),
        .is_loaded = false,
    };
    s_asset_registry[metadata.handle] = metadata;
  }
}