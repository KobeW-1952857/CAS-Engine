#pragma once

#include <glm/glm.hpp>

#include "renderer/primitives/line.h"
#include "renderer/primitives/point.h"
#include "scene/component_defaults.h"
#include "scene/components/arc_length_table.h"

struct LineComponent : ComponentDefaults {
  static constexpr std::string_view name = "Line";
  glm::vec3 p0 = {0.0f, 0.0f, 0.0f}, p1 = {0.0f, 0.0f, 0.0f};
  glm::vec3 color = {1.0f, 1.0f, 1.0f};
  float thickness = 1.0f;

  std::unique_ptr<LinePrimitive> line = std::make_unique<LinePrimitive>();
  std::unique_ptr<PointPrimitive> points = std::make_unique<PointPrimitive>();
  ArcLengthTable arc_length_table;

  static void serialize(YAML::Emitter& out, const LineComponent& c) {
    out << YAML::Key << "p0" << YAML::Value << YAML::Flow << c.p0;
    out << YAML::Key << "p1" << YAML::Value << YAML::Flow << c.p1;
    out << YAML::Key << "color" << YAML::Value << YAML::Flow << c.color;
    out << YAML::Key << "thickness" << YAML::Value << YAML::Flow << c.thickness;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    if (node["p0"] && node["p1"] && node["color"] && node["thickness"]) {
      auto& line = entity.addComponent<LineComponent>();
      line.p0 = node["p0"].as<glm::vec3>();
      line.p1 = node["p1"].as<glm::vec3>();
      line.line->update(line.p0, line.p1);
      line.points->update({line.p0, line.p1});
      line.color = node["color"].as<glm::vec3>();
      line.thickness = node["thickness"].as<float>();
      line.rebuildArcLengthTable();
    }
  }
  static void drawUI(LineComponent& c, AppContext& ctx) {
    if (ImGui::DragFloat3("P0", glm::value_ptr(c.p0))) {
      c.line->update(c.p0, c.p1);
      c.points->update({c.p0, c.p1});
      c.rebuildArcLengthTable();
    }
    if (ImGui::DragFloat3("P1", glm::value_ptr(c.p1))) {
      c.line->update(c.p0, c.p1);
      c.points->update({c.p0, c.p1});
      c.rebuildArcLengthTable();
    }
    ImGui::ColorEdit3("Color", glm::value_ptr(c.color));
    ImGui::DragFloat("Thickness", &c.thickness);
  }

  glm::vec3 evaluate(float t) const { return glm::mix(p0, p1, t); }
  float length() const { return arc_length_table.length(); }
  float tFromArcLength(float s) const { return arc_length_table.tFromArcLength(s); }
  void rebuildArcLengthTable() {
    arc_length_table.rebuild([&](float t) { return evaluate(t); });
  }
};