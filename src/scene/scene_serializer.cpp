#include "scene/scene_serializer.h"

#include <fstream>

#include "Nexus/Log.h"
#include "core/uuid.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "utils/yaml_extension.h"

bool SceneSerializer::deserialize(const std::string& filepath) {
  YAML::Node data;
  try {
    data = YAML::LoadFile(filepath);
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load scene file '{}'. Error: {}", filepath, e.what());
    return false;
  }

  if (!data["Scene"]) {
    Nexus::Logger::error("Invalid scene file '{}': Missing 'Scene' node.", filepath);
    return false;
  }

  m_scene->m_registry.clear();
  m_scene->m_entity_map.clear();

  std::string sceneName = data["Scene"].as<std::string>();
  Nexus::Logger::debug("Deserializing scene '{}'", sceneName);

  auto entities = data["Entities"];
  if (entities) {
    for (const auto& entity : entities) {
      uint64_t uuid = entity["ID"].as<uint64_t>();

      std::string name;
      auto tagComponent = entity["TagComponent"];
      if (tagComponent) {
        name = tagComponent["Tag"].as<std::string>();
      }

      Nexus::Logger::trace("Deserializing entity with ID = {}, Name = {}", uuid, name);
      Entity deserializedEntity = m_scene->createEntity(uuid, name);

      auto transformComponent = entity["TransformComponent"];
      if (transformComponent) {
        auto& tc = deserializedEntity.getComponent<TransformComponent>();
        tc.translation = transformComponent["Translation"].as<glm::vec3>();
        tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
        tc.scale = transformComponent["Scale"].as<glm::vec3>();
      }

      auto meshComponent = entity["MeshComponent"];
      if (meshComponent) {
        auto& mc = deserializedEntity.addComponent<MeshComponent>();
        mc.mesh_handle = meshComponent["MeshHandle"].as<UUID>();
      }
      auto materialComponent = entity["MaterialComponent"];
      if (materialComponent) {
        auto& mc = deserializedEntity.addComponent<MaterialComponent>();
        mc.material_handle = materialComponent["MaterialHandle"].as<UUID>();
      }
    }
  }

  Nexus::Logger::debug("Successfully deserialized scene '{}'", sceneName);
  return true;
}

void SceneSerializer::serialize(const std::string& filepath) {
  if (!m_scene) {
    Nexus::Logger::error("No scene to serialize");
    return;
  }
  Nexus::Logger::debug("Serializing scene to '{}", filepath);

  YAML::Emitter out;
  out << YAML::BeginMap;

  out << YAML::Key << "Scene" << YAML::Value << "Untitled scene";

  out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

  for (auto entity_id : m_scene->m_registry.view<entt::entity>()) {
    Entity entity = {entity_id, m_scene->weak_from_this()};
    if (!entity) continue;
    serializeEntity(out, entity);
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(filepath);
  fout << out.c_str();
}

void SceneSerializer::serializeEntity(YAML::Emitter& out, Entity entity) {
  out << YAML::BeginMap;
  out << YAML::Key << "ID" << YAML::Value << entity.getComponent<IDComponent>().ID;

  if (auto* tag = m_scene->m_registry.try_get<TagComponent>(entity)) {
    out << YAML::Key << "TagComponent" << YAML::BeginMap;
    out << YAML::Key << "Tag" << YAML::Value << tag->Tag;
    out << YAML::EndMap;
  }

  if (auto* transform = m_scene->m_registry.try_get<TransformComponent>(entity)) {
    out << YAML::Key << "TransformComponent" << YAML::BeginMap;
    out << YAML::Key << "Translation" << YAML::Value << transform->translation;
    out << YAML::Key << "Rotation" << YAML::Value << transform->rotation;
    out << YAML::Key << "Scale" << YAML::Value << transform->scale;
    out << YAML::EndMap;
  }

  if (auto* mesh_component = m_scene->m_registry.try_get<MeshComponent>(entity)) {
    out << YAML::Key << "MeshComponent" << YAML::BeginMap;
    out << YAML::Key << "MeshHandle" << YAML::Value << mesh_component->mesh_handle;
    out << YAML::EndMap;
  }

  if (auto* material_component = m_scene->m_registry.try_get<MaterialComponent>(entity)) {
    out << YAML::Key << "MaterialComponent" << YAML::BeginMap;
    out << YAML::Key << "MaterialHandle" << YAML::Value << material_component->material_handle;
    out << YAML::EndMap;
  }

  out << YAML::EndMap;
}
