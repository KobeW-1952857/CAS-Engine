#pragma once

#include "scene/component_defaults.h"
#include "scene/components/bezier_component.h"
#include "scene/components/id_component.h"
#include "scene/components/line_component.h"
#include "scene/components/tag_component.h"

struct LineFollowerComponent : ComponentDefaults {
  static constexpr std::string_view name = "Line Follower";

  UUID curve_entity;
  float speed = 1.0f;
  float t = 0.0f;
  bool loop = true;

  static void serialize(YAML::Emitter& out, const LineFollowerComponent& c) {
    out << YAML::Key << "line_entity" << YAML::Value << c.curve_entity;
    out << YAML::Key << "speed" << YAML::Value << c.speed;
    out << YAML::Key << "t" << YAML::Value << c.t;
    out << YAML::Key << "loop" << YAML::Value << c.loop;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    if (node["line_entity"] && node["speed"] && node["t"] && node["loop"]) {
      auto& c = entity.addComponent<LineFollowerComponent>();
      c.curve_entity = node["line_entity"].as<UUID>();
      c.speed = node["speed"].as<float>();
      c.t = node["t"].as<float>();
      c.loop = node["loop"].as<bool>();
    }
  }
  static void drawUI(LineFollowerComponent& c, AppContext& ctx) {
    ImGui::DragFloat("Speed", &c.speed, 0.1f, 0.0f, 100.0f);
    ImGui::SliderFloat("T", &c.t, 0.0f, 1.0f);
    ImGui::Checkbox("Loop", &c.loop);

    // Entity picker
    Entity line_entity = ctx.active_scene->getEntity(c.curve_entity);
    std::string line_name = line_entity ? line_entity.getComponent<TagComponent>().tag : "None";
    if (ImGui::BeginCombo("Line Entity", line_name.c_str())) {
      if (ImGui::Selectable("None", line_entity == Entity())) {
        c.curve_entity = UUID();
      }
      ctx.active_scene->forEachEntityWithAny<LineComponent, BezierComponent>([&](Entity e) {
        if (ImGui::Selectable(e.getComponent<TagComponent>().tag.c_str(), e == line_entity)) {
          c.curve_entity = e.getComponent<IDComponent>().ID;
        }
      });
      ImGui::EndCombo();
    }
  }
};