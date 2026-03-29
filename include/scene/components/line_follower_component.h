#pragma once

#include <imgui.h>

#include "editor/UI_components/bezier_curve_edit.h"
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
  float arc_length = 0.0f;
  bool loop = true;
  std::array<glm::vec2, 2> control_points = {glm::vec2(0.25f, 0.25f), glm::vec2(0.75f, 0.75f)};

  enum class Easing : uint8_t {
    Linear = 0,
    EaseIn,
    EaseOut,
    EaseInOut,
    EaseOutIn,
    Custom,
    COUNT
  } easing = Easing::Linear;
  static constexpr std::array<const char*, static_cast<size_t>(Easing::COUNT)> easing_names = {
      "Linear", "Ease In", "Ease Out", "Ease In-Out", "Ease Out-In", "Custom"};
  static constexpr std::array<const char*, static_cast<size_t>(Easing::COUNT)> easing_keys = {
      "linear", "ease_in", "ease_out", "ease_in_out", "ease_out_in", "custom"};
  static constexpr glm::vec2 easing_defaults[][2] = {
      [static_cast<int>(Easing::Linear)] = {{0.0f, 0.0f}, {1.0f, 1.0f}},
      [static_cast<int>(Easing::EaseIn)] = {{0.42f, 0.0f}, {1.0f, 1.0f}},
      [static_cast<int>(Easing::EaseOut)] = {{0.0f, 0.0f}, {0.58f, 1.0f}},
      [static_cast<int>(Easing::EaseInOut)] = {{0.42f, 0.0f}, {0.58f, 1.0f}},
      [static_cast<int>(Easing::EaseOutIn)] = {{0.42f, 1.0f}, {0.58f, 0.0f}},
      [static_cast<int>(Easing::Custom)] = {{0.25f, 0.25f}, {0.75f, 0.75f}}};

  static void serialize(YAML::Emitter& out, const LineFollowerComponent& c) {
    out << YAML::Key << "line_entity" << YAML::Value << c.curve_entity;
    out << YAML::Key << "speed" << YAML::Value << c.speed;
    out << YAML::Key << "loop" << YAML::Value << c.loop;
    out << YAML::Key << "easing" << YAML::Value << easing_keys[static_cast<int>(c.easing)];
    if (c.easing == Easing::Custom)

    {
      out << YAML::Key << "control_points" << YAML::Value << YAML::Flow << YAML::BeginSeq << YAML::BeginMap << YAML::Key
          << "x" << YAML::Value << c.control_points[0].x << YAML::Key << "y" << YAML::Value << c.control_points[0].y
          << YAML::EndMap << YAML::BeginMap << YAML::Key << "x" << YAML::Value << c.control_points[1].x << YAML::Key
          << "y" << YAML::Value << c.control_points[1].y << YAML::EndMap << YAML::EndSeq;
    }
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    if (!(node["line_entity"] && node["speed"] && node["loop"])) return;
    auto& c = entity.addComponent<LineFollowerComponent>();
    c.curve_entity = node["line_entity"].as<UUID>();
    c.speed = node["speed"].as<float>();
    c.loop = node["loop"].as<bool>();

    if (node["easing"]) {
      std::string easing_str = node["easing"].as<std::string>();
      auto it = std::find(easing_keys.begin(), easing_keys.end(), easing_str);
      if (it != easing_keys.end()) {
        c.easing = static_cast<Easing>(std::distance(easing_keys.begin(), it));
        if (c.easing != Easing::Custom)
          c.control_points = {easing_defaults[static_cast<int>(c.easing)][0],
                              easing_defaults[static_cast<int>(c.easing)][1]};
      }
      if (c.easing == Easing::Custom && node["control_points"] && node["control_points"].IsSequence() &&
          node["control_points"].size() == 2) {
        c.control_points[0] = {node["control_points"][0]["x"].as<float>(), node["control_points"][0]["y"].as<float>()};
        c.control_points[1] = {node["control_points"][1]["x"].as<float>(), node["control_points"][1]["y"].as<float>()};
      }
    }
  }

  static void drawUI(LineFollowerComponent& c, AppContext& ctx) {
    ImGui::DragFloat("Speed", &c.speed, 0.1f, 0.0f, 100.0f);
    ImGui::Checkbox("Loop", &c.loop);

    // Entity picker
    {
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

    // Easing function editor
    {
      if (ImGui::BeginCombo("Time-Distance function", easing_names[static_cast<int>(c.easing)])) {
        for (int i = 0; i < static_cast<int>(Easing::COUNT); ++i) {
          bool selected = (i == static_cast<int>(c.easing));
          if (ImGui::Selectable(easing_names[i], selected) && i != static_cast<int>(Easing::Custom)) {
            c.easing = static_cast<Easing>(i);
            c.control_points = {easing_defaults[i][0], easing_defaults[i][1]};
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImVec2 cp[2] = {{c.control_points[0].x, c.control_points[0].y}, {c.control_points[1].x, c.control_points[1].y}};

      if (ImGui::CurveEditBezier(ImVec2(0, 0), ImVec2(1, 1), cp, 2)) {
        c.control_points = {glm::vec2(cp[0].x, cp[0].y), glm::vec2(cp[1].x, cp[1].y)};
        c.easing = Easing::Custom;
      }
    }
  }
};