#pragma once

#include "renderer/primitives/line.h"
#include "renderer/primitives/point.h"
#include "scene/component_defaults.h"

struct LaticeComponent : ComponentDefaults {
  static constexpr std::string_view name = "Latice";

  int rows = 2;
  int cols = 2;
  int layers = 2;

  std::vector<glm::vec3> control_points;  // Generated at runtime, not serialized

  void generateVertices() {
    control_points.clear();
    float half_width = static_cast<float>(cols - 1) / 2.0f;
    float half_height = static_cast<float>(rows - 1) / 2.0f;
    float half_depth = static_cast<float>(layers - 1) / 2.0f;

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        for (int k = 0; k < layers; ++k) {
          control_points.emplace_back(static_cast<float>(j) - half_width, static_cast<float>(k) - half_depth,
                                      static_cast<float>(i) - half_height);
        }
      }
    }
  }

  static void serialize(YAML::Emitter& out, const LaticeComponent& c) {
    out << YAML::Key << "rows" << YAML::Value << c.rows;
    out << YAML::Key << "cols" << YAML::Value << c.cols;
    out << YAML::Key << "layers" << YAML::Value << c.layers;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    if (!(node["rows"] && node["cols"])) return;
    auto& c = entity.addComponent<LaticeComponent>();
    c.rows = node["rows"].as<int>();
    c.cols = node["cols"].as<int>();
    c.layers = node["layers"].as<int>();
    c.generateVertices();
  }

  static void drawUI(LaticeComponent& c, AppContext& ctx) {
    bool changed = ImGui::DragInt("Rows", &c.rows, 1, 1, 100);
    changed |= ImGui::DragInt("Cols", &c.cols, 1, 1, 100);
    changed |= ImGui::DragInt("Layers", &c.layers, 1, 1, 100);
    if (changed) c.generateVertices();
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::Button("Reset Vertices")) c.generateVertices();

    ImGui::Separator();
    if (c.control_points.size() > 0) {
      ImGui::Text("Vertex Count: %zu", c.control_points.size());
      if (ImGui::TreeNode("Vertices")) {
        for (size_t i = 0; i < c.control_points.size(); ++i) {
          const auto& v = c.control_points[i];
          ImGui::Text("Vertex %zu: (%.2f, %.2f, %.2f)", i, v.x, v.y, v.z);
        }
        ImGui::TreePop();
      }
    }
  }
};