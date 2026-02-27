#pragma once

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/asset.h"
#include "core/uuid.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

struct AssetMetadata {
  UUID handle;
  AssetType type;
  std::filesystem::path filepath;
  bool is_loaded = false;
};

class AssetManager {
 public:
  static void init();

  template <typename T>
  static std::vector<UUID> getAssetsOfType() {
    std::vector<UUID> assets;
    auto type = getAssetType<T>();

    for (auto [id, meta] : s_asset_registry)
      if (meta.type == type) assets.push_back(id);

    return assets;
  }

  template <typename T>
  static std::unordered_map<UUID, AssetMetadata> getAssetsMetadataOfType() {
    std::unordered_map<UUID, AssetMetadata> assets;
    auto type = getAssetType<T>();

    for (auto [id, meta] : s_asset_registry)
      if (meta.type == type) assets.emplace(id, meta);

    return assets;
  }

  template <typename T>
  static std::shared_ptr<T> getAsset(UUID handle) {
    if (!s_asset_registry.contains(handle)) return nullptr;
    if (s_loaded_assets.contains(handle)) return std::dynamic_pointer_cast<T>(s_loaded_assets[handle]);

    return loadAsset<T>(handle);
  }

  static const AssetMetadata& getAssetMetadata(UUID handle) { return s_asset_registry[handle]; }

  static void addMemoryOnlyAsset(std::shared_ptr<Asset> asset, std::string filepath = "Memory Only") {
    if (!asset) return;
    if (!asset->handle) asset->handle = UUID();

    s_loaded_assets[asset->handle] = asset;
    AssetMetadata meta{.handle = asset->handle, .is_loaded = true, .filepath = filepath, .type = asset->type};
    s_asset_registry[asset->handle] = meta;
  }

  static void serialize(YAML::Emitter& out);
  static void deserialize(const YAML::Node& node);

 public:
 private:
  template <typename T>
  static std::shared_ptr<T> loadAsset(UUID handle) {
    const auto& metadata = s_asset_registry[handle];
    std::shared_ptr<T> asset = nullptr;

    if constexpr (std::is_same_v<T, Texture>) {
      asset = std::make_shared<Texture>(metadata.filepath.string());
    } else if constexpr (std::is_same_v<T, Mesh>) {
      asset = std::make_shared<Mesh>(metadata.filepath.string());
    } else if constexpr (std::is_same_v<T, Shader>) {
      std::string vertPath = metadata.filepath.string() + ".vert";
      std::string fragPath = metadata.filepath.string() + ".frag";
      asset = std::make_shared<Shader>(vertPath.c_str(), fragPath.c_str());
    } else if constexpr (std::is_same_v<T, Material>) {
      asset = std::make_shared<Material>();
    }

    if (!asset) return nullptr;
    asset->handle = handle;
    s_loaded_assets[handle] = asset;
    s_asset_registry[handle].is_loaded = true;
    return std::dynamic_pointer_cast<T>(asset);
  }

  template <typename T>
  static AssetType getAssetType() {
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

 private:
  inline static uint64_t s_ENGINE_ASSETS = 0;
  inline static std::unordered_map<UUID, AssetMetadata> s_asset_registry;
  inline static std::unordered_map<UUID, std::shared_ptr<Asset>> s_loaded_assets;
};