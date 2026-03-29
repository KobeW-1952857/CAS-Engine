#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include "core/asset.h"
#include "core/project.h"
#include "core/uuid.h"

class FileSystem;
class Renderer;

struct AssetMetadata {
  UUID handle;
  AssetType type;
  std::filesystem::path filepath;
  bool is_loaded = false;
  bool is_engine_asset = false;
};

class AssetManager {
 public:
  explicit AssetManager(FileSystem& filesystem) : m_filesystem(filesystem) {}
  void init();

  template <typename T>
  std::vector<UUID> getAssetsOfType();

  template <typename T>
  std::unordered_map<UUID, AssetMetadata> getAssetsMetadataOfType();
  UUID getHandleFromPath(const std::filesystem::path& path);
  AssetMetadata& getAssetMetadata(UUID handle);
  void renameAsset(UUID handle, const std::filesystem::path& new_path);

  template <typename T>
  std::shared_ptr<T> getAsset(UUID handle);
  template <typename T>
  std::shared_ptr<T> getDefaultAsset();

  template <typename T>
  UUID createNewAsset(const std::filesystem::path& virtual_directory = "project://");

  void serialize();
  void saveAssets();
  void deserialize(const std::filesystem::path& path);

  void syncFileSystem();

  FileSystem& filesystem() { return m_filesystem; }
  Renderer* getRenderer() { return m_renderer; }
  void setRenderer(Renderer* renderer) { m_renderer = renderer; }

 private:
  template <typename T>
  std::shared_ptr<T> loadAsset(UUID handle);

  AssetType getTypeFromExtension(std::string_view ext) const;

  template <typename T>
  static AssetType getAssetType();
  void registerEngineAssets();

  uint64_t parseRegistry(const std::filesystem::path& path);

 private:
  uint64_t s_ENGINE_ASSETS = 0;
  std::unordered_map<UUID, AssetMetadata> s_asset_registry;
  std::unordered_map<UUID, std::shared_ptr<Asset>> s_loaded_assets;
  std::unordered_map<AssetType, UUID> s_default_assets;

  FileSystem& m_filesystem;
  Renderer* m_renderer;
};

#include "asset_manager.inl"
