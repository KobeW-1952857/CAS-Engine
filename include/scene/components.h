#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

#include "core/uuid.h"

struct IDComponent {
  static constexpr std::string name = "ID";
  UUID ID;

  IDComponent() = default;
  IDComponent(UUID id) : ID(id) {}
  IDComponent(const IDComponent&) = default;
};

struct TagComponent {
  static constexpr std::string name = "Tag";
  std::string Tag;

  TagComponent() = default;
  TagComponent(const std::string& tag) : Tag(tag) {}
  TagComponent(const TagComponent&) = default;
};

struct TransformComponent {
  static constexpr std::string name = "Transform";
  glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
  glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

  TransformComponent() = default;
  TransformComponent(const TransformComponent&) = default;
  TransformComponent(const glm::vec3& translation) : Translation(translation) {}

  glm::mat4 getTransform() const {
    glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

    return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
  }
};

struct MeshComponent {
  static constexpr std::string name = "Mesh";
  UUID mesh_handle = 3;

  MeshComponent() = default;
};

struct MaterialComponent {
  static constexpr std::string name = "Material";
  UUID material_handle = 2;

  MaterialComponent() = default;
};