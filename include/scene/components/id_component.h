#pragma once

#include <yaml-cpp/yaml.h>

#include <string_view>

#include "core/uuid.h"
#include "scene/component_defaults.h"
#include "scene/entity.h"

struct AppContext;

struct IDComponent : ComponentDefaults {
  static constexpr std::string_view name = "ID";
  static constexpr bool user_addable = false;
  static constexpr bool user_removable = false;
  static constexpr bool user_editable = false;

  UUID ID;

  IDComponent() = default;
  explicit IDComponent(UUID id) : ID(id) {}

  static void serialize(YAML::Emitter& out, const IDComponent& c) { out << YAML::Key << "ID" << YAML::Value << c.ID; }
  static void deserialize(const YAML::Node& node, Entity entity) {}
  static void drawUI(IDComponent& c, AppContext& ctx) {}
};