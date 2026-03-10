#pragma once

#include <filesystem>
#include <string>

struct ProjectConfig {
  std::string name;
  std::filesystem::path path;
  std::filesystem::path assets;
  std::filesystem::path start_scene;
};

class Project {
 public:
  static void load();
  static void load(const std::filesystem::path& path);
  static void save();
  static void New();

  static ProjectConfig& getConfig() { return s_config; }
  static std::filesystem::path getProjectPath() { return s_config.path; }
  static std::filesystem::path getAssetsPath() { return s_config.path / "assets"; }

  static bool hasProject() { return std::filesystem::exists(s_config.path / (s_config.name + ".cproj")); }

 private:
  static bool serialize(const std::filesystem::path& path);
  static bool deserialize(const std::filesystem::path& path);

 private:
  inline static ProjectConfig s_config = {};
};
