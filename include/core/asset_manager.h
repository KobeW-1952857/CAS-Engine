#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include "core/asset.h"
#include "core/project.h"
#include "core/uuid.h"

struct AssetMetadata {
  UUID handle;
  AssetType type;
  std::filesystem::path filepath;
  bool is_loaded = false;
  bool is_engine_asset = false;
};

class AssetManager {
 public:
  static void init();

  template <typename T>
  static std::vector<UUID> getAssetsOfType();
  template <typename T>
  static std::unordered_map<UUID, AssetMetadata> getAssetsMetadataOfType();
  static UUID getHandleFromPath(const std::filesystem::path& path);
  static AssetMetadata& getAssetMetadata(UUID handle);

  template <typename T>
  static std::shared_ptr<T> getAsset(UUID handle);
  template <typename T>
  static std::shared_ptr<T> getDefaultAsset();

  template <typename T>
  static UUID createNewAsset(const std::filesystem::path& virtual_directory = "project://");

  static void serialize();
  static void saveAssets();
  static void deserialize(const std::filesystem::path& path = Project::getConfig().path / "assets.cas");

  static void syncFileSystem();

 private:
  template <typename T>
  static std::shared_ptr<T> loadAsset(UUID handle);

  template <typename T>
  static AssetType getAssetType();
  static void registerEngineAssets();

  static uint64_t parseRegistry(const std::filesystem::path& path);

 private:
  inline static uint64_t s_ENGINE_ASSETS = 0;
  inline static std::unordered_map<UUID, AssetMetadata> s_asset_registry;
  inline static std::unordered_map<UUID, std::shared_ptr<Asset>> s_loaded_assets;
  inline static std::unordered_map<AssetType, UUID> s_default_assets;
};

#include "asset_manager.inl"
