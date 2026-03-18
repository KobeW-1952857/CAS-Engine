#pragma once

#include <imgui.h>

#include <array>

#include "renderer/primitives/patch.h"
#include "renderer/primitives/point.h"
#include "scene/component_defaults.h"
#include "scene/components/arc_length_table.h"

struct BezierComponent : ComponentDefaults {
  static constexpr std::string_view name = "Bezier";

  std::array<glm::vec3, 4> control_points{glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f},
                                          glm::vec3{1.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}};

  glm::vec3 color = {1.0f, 1.0f, 1.0f};

  std::unique_ptr<PatchPrimitive> patch = std::make_unique<PatchPrimitive>(4);
  std::unique_ptr<PointPrimitive> points = std::make_unique<PointPrimitive>();
  ArcLengthTable arc_length_table;

  static void serialize(YAML::Emitter& out, const BezierComponent& c) {
    for (int i = 0; i < 4; ++i) {
      out << YAML::Key << "p" + std::to_string(i) << YAML::Value << YAML::Flow << c.control_points[i];
    }
    out << YAML::Key << "color" << YAML::Value << YAML::Flow << c.color;
  }

  static void deserialize(const YAML::Node& node, Entity entity) {
    if (!(node["p0"] && node["p1"] && node["p2"] && node["p3"] && node["color"])) return;

    auto& bezier = entity.addComponent<BezierComponent>();
    bezier.control_points[0] = node["p0"].as<glm::vec3>();
    bezier.control_points[1] = node["p1"].as<glm::vec3>();
    bezier.control_points[2] = node["p2"].as<glm::vec3>();
    bezier.control_points[3] = node["p3"].as<glm::vec3>();
    bezier.patch->update(
        {bezier.control_points[0], bezier.control_points[1], bezier.control_points[2], bezier.control_points[3]});
    bezier.points->update(
        {bezier.control_points[0], bezier.control_points[1], bezier.control_points[2], bezier.control_points[3]});
    bezier.color = node["color"].as<glm::vec3>();
    bezier.rebuildArcLengthTable();
  }
  static void drawUI(BezierComponent& c, AppContext& ctx) {
    for (int i = 0; i < 4; ++i) {
      if (ImGui::DragFloat3(("P" + std::to_string(i)).c_str(), glm::value_ptr(c.control_points[i]))) {
        c.patch->update({c.control_points[0], c.control_points[1], c.control_points[2], c.control_points[3]});
        c.points->update({c.control_points[0], c.control_points[1], c.control_points[2], c.control_points[3]});
        c.rebuildArcLengthTable();
      }
    }
    ImGui::ColorEdit3("Color", glm::value_ptr(c.color));
    ImGui::Separator();
    c.arc_length_table.drawUI([&c](float t) { return c.evaluate(t); });
  }

  glm::vec3 evaluate(float t) const {
    float u = 1.0f - t;
    return u * u * u * control_points[0] + 3.0f * u * u * t * control_points[1] + 3.0f * u * t * t * control_points[2] +
           t * t * t * control_points[3];
  }
  float length() const { return arc_length_table.length(); }

  float tFromArcLength(float s) const { return arc_length_table.tFromArcLength(s); }

  void rebuildArcLengthTable() {
    arc_length_table.rebuild([&](float t) { return evaluate(t); });
  }

 private:
};