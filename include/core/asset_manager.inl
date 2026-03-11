#pragma once

#include <string>

#include "core/asset_manager.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "utils/filesystem.h"

template <typename T>
std::vector<UUID> AssetManager::getAssetsOfType() {
  std::vector<UUID> assets;
  auto type = getAssetType<T>();

  for (auto [id, meta] : s_asset_registry)
    if (meta.type == type) assets.push_back(id);

  return assets;
}

template <typename T>
std::unordered_map<UUID, AssetMetadata> AssetManager::getAssetsMetadataOfType() {
  std::unordered_map<UUID, AssetMetadata> assets;
  auto type = getAssetType<T>();

  for (auto [id, meta] : s_asset_registry)
    if (meta.type == type) assets.emplace(id, meta);

  return assets;
}

template <typename T>
std::shared_ptr<T> AssetManager::getAsset(UUID handle) {
  if (!s_asset_registry.contains(handle)) return nullptr;
  if (s_loaded_assets.contains(handle)) return std::dynamic_pointer_cast<T>(s_loaded_assets[handle]);

  return loadAsset<T>(handle);
}

template <typename T>
std::shared_ptr<T> AssetManager::getDefaultAsset() {
  if (!s_default_assets.contains(getAssetType<T>())) return nullptr;
  return getAsset<T>(s_default_assets[getAssetType<T>()]);
}

template <typename T>
UUID AssetManager::createNewAsset(const std::filesystem::path& virtual_directory) {
  static const char* base_name = "New Asset";
  static const char* extension = ".casset";

  if constexpr (std::is_same_v<T, Material>) {
    base_name = "New Material";
    extension = ".casmat";
  }
  // TODO(Kobe): Add all other asset types

  std::filesystem::path virtual_path = virtual_directory / std::string(base_name).append(extension);
  std::filesystem::path absolute_path = FileSystem::resolvePath(virtual_path);

  int counter = 1;
  while (std::filesystem::exists(absolute_path)) {
    virtual_path = virtual_directory / (std::string(base_name) + "_" + std::to_string(counter).append(extension));
    absolute_path = FileSystem::resolvePath(virtual_path);
    counter++;
  }

  auto new_asset = std::make_shared<T>();
  new_asset->serialize(FileSystem::resolvePath(virtual_path));

  UUID new_id;
  new_asset->handle = new_id;

  AssetMetadata meta{
      .handle = new_id,
      .is_loaded = true,
      .filepath = virtual_path,
      .type = getAssetType<T>(),
  };

  Nexus::Logger::debug("Creating new asset at {}", meta.filepath.string());
  s_asset_registry[new_id] = meta;
  s_loaded_assets[new_id] = new_asset;

  serialize();

  return new_id;
}

template <typename T>
std::shared_ptr<T> AssetManager::loadAsset(UUID handle) {
  const auto& metadata = s_asset_registry[handle];
  std::shared_ptr<T> asset = nullptr;

  auto path = FileSystem::resolvePath(metadata.filepath);

  if constexpr (std::is_same_v<T, Texture>) {
    asset = std::make_shared<Texture>(path.string());
  } else if constexpr (std::is_same_v<T, Mesh>) {
    asset = std::make_shared<Mesh>(path.string());
  } else if constexpr (std::is_same_v<T, Shader>) {
    asset = std::make_shared<Shader>(path);
  } else if constexpr (std::is_same_v<T, Material>) {
    asset = Material::load(path.string());
  }

  if (!asset) return nullptr;
  asset->handle = handle;
  s_loaded_assets[handle] = asset;
  s_asset_registry[handle].is_loaded = true;
  return std::dynamic_pointer_cast<T>(asset);
}

template <typename T>
AssetType AssetManager::getAssetType() {
  if constexpr (std::is_same_v<T, Texture>) {
    return AssetType::Texture;
  } else if constexpr (std::is_same_v<T, Mesh>) {
    return AssetType::Mesh;
  } else if constexpr (std::is_same_v<T, Shader>) {
    return AssetType::Shader;
  } else if constexpr (std::is_same_v<T, Material>) {
    return AssetType::Material;
  } else {
    return AssetType::None;
  }
}
