#pragma once
#include <vector>

#include "scene/component_defaults.h"

struct ChildrenComponent : ComponentDefaults {
  static constexpr std::string_view name = "Children";
  static constexpr bool user_addable = false;
  static constexpr bool user_removable = false;
  static constexpr bool user_editable = false;

  std::vector<UUID> children;

  static void serialize(YAML::Emitter& out, const ChildrenComponent& c) {
    out << YAML::Key << "ChildrenComponent" << YAML::BeginMap;
    out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
    for (const auto& id : c.children) out << id;
    out << YAML::EndSeq;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["ChildrenComponent"];
    if (!n) return;
    auto& cc = entity.addComponent<ChildrenComponent>();
    for (const auto& child : n["Children"]) cc.children.push_back(child.as<UUID>());
  }
  static void drawUI(ChildrenComponent& c, AppContext& ctx) {}
};