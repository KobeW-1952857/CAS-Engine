#pragma once

#include <filesystem>
#include <string>

class FileSystem;
class AssetManager;

struct ProjectConfig {
  std::string name;
  std::filesystem::path path;
  std::filesystem::path assets;
  std::filesystem::path start_scene;
};

class Project {
 public:
  Project(FileSystem& filesystem, AssetManager& assets) : m_filesystem(filesystem), m_assets(assets) {}

  void load();
  void load(const std::filesystem::path& path);
  void save();
  void New();

  ProjectConfig& getConfig() { return m_config; }
  const ProjectConfig& getConfig() const { return m_config; }
  std::filesystem::path getProjectPath() const { return m_config.path; }
  std::filesystem::path getAssetsPath() const { return m_config.path / "assets"; }

  bool hasProject() const { return m_has_project; }

 private:
  bool serialize(const std::filesystem::path& path);
  bool deserialize(const std::filesystem::path& path);

 private:
  ProjectConfig m_config = {};

  FileSystem& m_filesystem;
  AssetManager& m_assets;
  bool m_has_project = false;
};
