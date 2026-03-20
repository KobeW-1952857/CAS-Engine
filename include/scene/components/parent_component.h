#pragma once

#include "scene/component_defaults.h"

struct ParentComponent : ComponentDefaults {
  static constexpr std::string_view name = "Parent";
  static constexpr bool user_addable = false;
  static constexpr bool user_removable = false;
  static constexpr bool user_editable = false;

  UUID parent_id;

  static void serialize(YAML::Emitter& out, const ParentComponent& c) {
    out << YAML::Key << "ParentComponent" << YAML::BeginMap;
    out << YAML::Key << "ParentID" << YAML::Value << c.parent_id;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["ParentComponent"];
    if (!n) return;
    auto& pc = entity.addComponent<ParentComponent>();
    pc.parent_id = n["ParentID"].as<UUID>();
  }
  static void drawUI(ParentComponent& c, AppContext& ctx) {}
};