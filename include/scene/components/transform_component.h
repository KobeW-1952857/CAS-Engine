#pragma once

#include <imgui.h>
#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string_view>

#include "core/app_context.h"
#include "scene/component_defaults.h"
#include "scene/entity.h"
#include "utils/yaml_extension.h"

struct TransformComponent : ComponentDefaults {
  static constexpr std::string_view name = "Transform";
  static constexpr bool user_addable = false;
  static constexpr bool user_removable = false;

  glm::vec3 translation = {0, 0, 0};
  glm::vec3 rotation = {0, 0, 0};
  glm::vec3 scale = {1, 1, 1};

  glm::mat4 getTransform() const {
    return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(glm::quat(rotation)) *
           glm::scale(glm::mat4(1.0f), scale);
  }

  static void serialize(YAML::Emitter& out, const TransformComponent& c) {
    out << YAML::Key << "TransformComponent" << YAML::BeginMap;
    out << YAML::Key << "Translation" << YAML::Value << c.translation;
    out << YAML::Key << "Rotation" << YAML::Value << c.rotation;
    out << YAML::Key << "Scale" << YAML::Value << c.scale;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, Entity& entity) {
    auto n = node["TransformComponent"];
    if (!n) return;
    auto& tc = entity.getComponent<TransformComponent>();
    tc.translation = n["Translation"].as<glm::vec3>();
    tc.rotation = n["Rotation"].as<glm::vec3>();
    tc.scale = n["Scale"].as<glm::vec3>();
  }
  static void drawUI(TransformComponent& c, AppContext& ctx) {
    ImGui::DragFloat3("Translation", glm::value_ptr(c.translation));
    ImGui::DragFloat3("Rotation", glm::value_ptr(c.rotation));
    ImGui::DragFloat3("Scale", glm::value_ptr(c.scale));
  }
};