#pragma once

#include <ImGui/imgui.h>
#include <ImGui/misc/cpp/imgui_stdlib.h>
#include <yaml-cpp/yaml.h>

#include <string>
#include <string_view>

#include "core/app_context.h"
#include "scene/component_defaults.h"
#include "scene/entity.h"

struct TagComponent : ComponentDefaults {
  static constexpr std::string_view name = "Tag";
  static constexpr bool user_addable = false;
  static constexpr bool user_removable = false;
  static constexpr bool user_editable = false;

  std::string tag;

  TagComponent() = default;
  explicit TagComponent(const std::string& tag) : tag(tag) {}

  static void serialize(YAML::Emitter& out, const TagComponent& c) {
    out << YAML::Key << "TagComponent" << YAML::BeginMap;
    out << YAML::Key << "Tag" << YAML::Value << c.tag;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {}
  static void drawUI(TagComponent& c, AppContext& ctx) { ImGui::InputText("##tag", &c.tag); }
};