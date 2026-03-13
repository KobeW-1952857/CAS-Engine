#pragma once

#include "scene/components.h"
#include "scene/entity.h"
#include "utils/yaml_extension.h"

template <typename T>
struct ComponentTraits;

template <>
struct ComponentTraits<BezierCurveComponent> {
  static void serialize(YAML::Emitter& out, const Entity& entity) {
    if (auto* bc = entity.tryGetComponent<BezierCurveComponent>()) {
      out << YAML::Key << "BezierCurveComponent" << YAML::BeginMap;
      for (int i = 0; i < 4; i++) {
        out << YAML::Key << ("P" + std::to_string(i)) << YAML::Value << bc->control_points[i];
      }
      out << YAML::EndMap;
    }
  }

  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["BezierCurveComponent"];
    if (n) {
      auto& bc = entity.addComponent<BezierCurveComponent>();
      for (int i = 0; i < 4; i++) {
        bc.control_points[i] = n[("P" + std::to_string(i))].as<glm::vec3>();
      }
    }
  }
};