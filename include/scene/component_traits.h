#pragma once

#include "scene/components.h"
#include "scene/entity.h"
#include "utils/yaml_extension.h"

template <typename T>
struct ComponentTraits;

template <>
struct ComponentTraits<IDComponent> {
  static void serialize(YAML::Emitter& out, const Entity& entity) {
    const auto& id = entity.getComponent<IDComponent>().ID;
    out << YAML::Key << "ID" << YAML::Value << id;
  }
  // Handled directly by scene, needed to create the entitiy in the registry
  static void deserialize(const YAML::Node& node, Entity& entity) {}
};

template <>
struct ComponentTraits<TagComponent> {
  static void serialize(YAML::Emitter& out, const Entity& entity) {
    const auto& tc = entity.getComponent<TagComponent>();
    out << YAML::Key << "TagComponent" << YAML::BeginMap;
    out << YAML::Key << "Tag" << YAML::Value << tc.Tag;
    out << YAML::EndMap;
  }
  // Handled directly by scene, needed to create the entitiy in the registry
  static void deserialize(const YAML::Node& node, Entity& entity) {}
};

template <>
struct ComponentTraits<TransformComponent> {
  static void serialize(YAML::Emitter& out, const Entity& entity) {
    auto& tc = entity.getComponent<TransformComponent>();
    out << YAML::Key << "TransformComponent" << YAML::BeginMap;
    out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
    out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
    out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, const Entity& entity) {
    auto n = node["TransformComponent"];
    if (!n) return;
    auto& tc = entity.getComponent<TransformComponent>();
    tc.Translation = n["Translation"].as<glm::vec3>();
    tc.Rotation = n["Rotation"].as<glm::vec3>();
    tc.Scale = n["Scale"].as<glm::vec3>();
  }
};

template <>
struct ComponentTraits<MeshComponent> {
  static void serialize(YAML::Emitter& out, const Entity& entity) {
    if (auto* mc = entity.tryGetComponent<MeshComponent>()) {
      out << YAML::Key << "MeshComponent" << YAML::BeginMap;
      out << YAML::Key << "MeshHandle" << YAML::Value << mc->mesh_handle;
      out << YAML::EndMap;
    }
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["MeshComponent"];
    if (n) {
      auto& mc = entity.addComponent<MeshComponent>();
      mc.mesh_handle = n["MeshHandle"].as<UUID>();
    }
  }
};

template <>
struct ComponentTraits<MaterialComponent> {
  static void serialize(YAML::Emitter& out, const Entity& entity) {
    if (auto* mc = entity.tryGetComponent<MaterialComponent>()) {
      out << YAML::Key << "MaterialComponent" << YAML::BeginMap;
      out << YAML::Key << "MaterialHandle" << YAML::Value << mc->material_handle;
      out << YAML::EndMap;
    }
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["MaterialComponent"];
    if (n) {
      auto& mc = entity.addComponent<MaterialComponent>();
      mc.material_handle = n["MaterialHandle"].as<UUID>();
    }
  }
};