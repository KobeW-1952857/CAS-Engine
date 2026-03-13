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
  m_filesystem.setProjectRoot(m_config.path);
  m_assets.init();
  m_has_project = true;
}

void Project::save() {
  serialize(m_config.path / (m_config.name + ".cproj"));
  m_assets.saveAssets();
  Nexus::Logger::info("Successfully saved project '{}'", m_config.path.string());
}

void Project::New() {
  auto save_path = std::filesystem::path(FileDialog::saveFile());
  if (save_path.empty()) return;
  m_config.path = save_path.parent_path();
  m_config.name = save_path.stem();
  serialize(save_path);
  m_filesystem.setProjectRoot(m_config.path);
  m_has_project = true;
  Nexus::Logger::info("Successfully saved project '{}'", save_path.string());
}

bool Project::serialize(const std::filesystem::path& path) {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Project" << YAML::Value << YAML::BeginMap;
  out << YAML::Key << "Name" << YAML::Value << m_config.name;
  out << YAML::Key << "Path" << YAML::Value << m_config.path;
  out << YAML::Key << "StartScene" << YAML::Value << m_config.start_scene;
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

  m_config.name = node["Project"]["Name"].as<std::string>();
  m_config.path = node["Project"]["Path"].as<std::string>();
  m_config.start_scene = node["Project"]["StartScene"].as<UUID>();

  return true;
}