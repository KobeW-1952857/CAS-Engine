#include "core/project.h"

#include <Nexus/Log.h>
#include <yaml-cpp/yaml.h>

#include <fstream>

#include "core/asset_manager.h"
#include "utils/file_dialog.h"
#include "utils/filesystem.h"
#include "yaml-cpp/emittermanip.h"

void Project::load() {
  auto path = FileDialog::openFile();
  Nexus::Logger::debug("Loading project {}...", path);
  if (!path.empty()) load(path);
}
void Project::load(const std::filesystem::path& path) {
  if (!deserialize(path)) {
    Nexus::Logger::error("Failed to load project '{}'", path.string());
    return;
  }
  Nexus::Logger::info("Successfully loaded project '{}'", path.string());
  FileSystem::s_project_root = s_config.path;
  AssetManager::init();
}

void Project::save() {
  serialize(s_config.path / (s_config.name + ".cproj"));
  AssetManager::saveAssets();
  Nexus::Logger::info("Successfully saved project '{}'", s_config.path.string());
}

void Project::New() {
  auto save_path = std::filesystem::path(FileDialog::saveFile());
  if (save_path.empty()) return;
  s_config.path = save_path.parent_path();
  s_config.name = save_path.stem();
  serialize(save_path);
  Nexus::Logger::info("Successfully saved project '{}'", save_path.string());
}

bool Project::serialize(const std::filesystem::path& path) {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Project" << YAML::Value << YAML::BeginMap;
  out << YAML::Key << "Name" << YAML::Value << s_config.name;
  out << YAML::Key << "Path" << YAML::Value << s_config.path;
  out << YAML::Key << "StartScene" << YAML::Value << s_config.start_scene;
  out << YAML::EndMap;
  out << YAML::EndMap;

  std::ofstream fout(path);
  fout << out.c_str();
  return true;
}

bool Project::deserialize(const std::filesystem::path& path) {
  YAML::Node node;
  try {
    node = YAML::LoadFile(path);
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load scene file '{}'. Error: {}", path.string(), e.what());
    return false;
  }

  if (!node["Project"]) {
    Nexus::Logger::error("Invalid scene file '{}': Missing 'Project' node.", path.string());
    return false;
  }

  s_config.name = node["Project"]["Name"].as<std::string>();
  s_config.path = node["Project"]["Path"].as<std::string>();
  s_config.start_scene = node["Project"]["StartScene"].as<std::string>();

  return true;
}