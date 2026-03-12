#include "core/asset_manager.h"

#include <Nexus/Log.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>

#include "core/asset.h"
#include "core/project.h"
#include "core/uuid.h"
#include "utils/filesystem.h"

void AssetManager::init() {
  s_asset_registry.clear();
  s_loaded_assets.clear();
  s_ENGINE_ASSETS = 0;
  registerEngineAssets();

  if (std::filesystem::exists(m_filesystem.resolvePath("project://assets.casreg")))
    deserialize(m_filesystem.resolvePath("project://assets.casreg"));

  syncFileSystem();
}

void AssetManager::registerEngineAssets() {
  auto path = m_filesystem.resolvePath("engine://assets.casreg");
  s_ENGINE_ASSETS = parseRegistry(path);
  Nexus::Logger::info("Loaded {} engine assets", s_ENGINE_ASSETS);
}

void AssetManager::serialize() {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Assets";
  out << YAML::Value << YAML::BeginSeq;

  for (const auto& [id, meta] : s_asset_registry) {
    if (meta.is_engine_asset) continue;

    out << YAML::BeginMap;
    out << YAML::Key << "ID" << YAML::Value << id;
    out << YAML::Key << "Type" << YAML::Value << static_cast<int>(meta.type);
    out << YAML::Key << "Filepath" << YAML::Value << meta.filepath;
    out << YAML::EndMap;
  }

  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(m_filesystem.resolvePath("project://assets.casreg"));
  fout << out.c_str();
}

void AssetManager::saveAssets() {
  for (const auto& [id, asset] : s_loaded_assets) {
    if (asset->modified) {
      asset->modified = false;
      auto meta = getAssetMetadata(id);
      if (meta.is_engine_asset) continue;
      asset->serialize(m_filesystem.resolvePath(meta.filepath));
    }
  }
}

void AssetManager::deserialize(const std::filesystem::path& path) {
  Nexus::Logger::debug("Deserializing assets from {}...", path.string());
  if (path.filename() != "assets.casreg") return;

  parseRegistry(path);
}

uint64_t AssetManager::parseRegistry(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    Nexus::Logger::warn("Registry file not found at {}", path.string());
    return 0;
  }

  YAML::Node node;
  try {
    node = YAML::LoadFile(path.string());
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load registry file '{}'. Error: {}", path.string(), e.what());
    return 0;
  }

  auto registry_node = node["Assets"];
  if (!registry_node) return 0;

  uint64_t highest_id = 0;

  for (const auto& asset : registry_node) {
    AssetMetadata metadata;
    metadata.handle = asset["ID"].as<UUID>();
    metadata.type = asset["Type"].as<AssetType>();
    metadata.filepath = asset["Filepath"].as<std::string>();
    metadata.is_loaded = false;

    if (asset["Default"] && asset["Default"].as<bool>()) s_default_assets[metadata.type] = metadata.handle;
    if (asset["Filepath"].as<std::string>().starts_with("engine://")) metadata.is_engine_asset = true;

    s_asset_registry[metadata.handle] = metadata;

    if (metadata.handle > highest_id) {
      highest_id = metadata.handle;
    }
  }

  Nexus::Logger::trace("Loaded {} assets from {}", registry_node.size(), path.filename().string());
  return highest_id;
}

UUID AssetManager::getHandleFromPath(const std::filesystem::path& path) {
  std::string search_path = path.string();
  for (const auto& [uuid, metadata] : s_asset_registry) {
    if (metadata.filepath == search_path) return uuid;
  }
  return UUID(0);
}

AssetMetadata& AssetManager::getAssetMetadata(UUID handle) { return s_asset_registry[handle]; }

void AssetManager::syncFileSystem() {
  auto project_path = m_filesystem.getProjectRoot();
  if (!std::filesystem::exists(project_path)) return;
  Nexus::Logger::info("Syncing file system...");

  std::unordered_set<std::string> files_on_disk;

  std::unordered_map<std::string, UUID> path_to_uuid;
  for (const auto& [id, meta] : s_asset_registry) path_to_uuid[meta.filepath.string()] = id;

  bool registry_modified = false;

  // Scan disk for new assets
  for (const auto& entry : std::filesystem::recursive_directory_iterator(project_path)) {
    if (!entry.is_regular_file()) continue;

    std::string file_path = m_filesystem.getProjectPath(entry.path()).string();
    std::string extension = entry.path().extension().string();

    files_on_disk.insert(file_path);
    if (!path_to_uuid.contains(file_path)) {
      UUID new_id;
      AssetType type = AssetType::None;
      if (extension == ".casmat") type = AssetType::Material;
      if (extension == ".obj") type = AssetType::Mesh;
      if (extension == ".frag") type = AssetType::Shader;
      if (extension == ".vert") type = AssetType::Shader;

      if (type == AssetType::None) continue;

      AssetMetadata meta{
          .handle = new_id,
          .type = type,
          .filepath = file_path,
      };
      s_asset_registry[new_id] = meta;
      registry_modified = true;

      Nexus::Logger::info("Auto-discovered new asset: {}, {}", static_cast<uint64_t>(meta.handle),
                          meta.filepath.string());
    }
  }

  // Scan registry for stale assets
  std::vector<UUID> assets_to_remove;
  for (const auto& [id, meta] : s_asset_registry) {
    if (!files_on_disk.contains(meta.filepath.string()) && meta.filepath.string() != "project://assets.casreg" &&
        meta.filepath.string().starts_with("project://")) {
      assets_to_remove.push_back(id);
      registry_modified = true;
    }
  }

  // Remove stale assets
  for (const auto& id : assets_to_remove) {
    Nexus::Logger::info("Removed stale asset: {}", s_asset_registry[id].filepath.string());
    s_asset_registry.erase(id);
    s_loaded_assets.erase(id);
    registry_modified = true;
  }

  // Save registry if needed
  if (registry_modified) serialize();
}