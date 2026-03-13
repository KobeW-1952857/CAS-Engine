#pragma once

#include <imgui.h>

#include <string_view>

#include "core/app_context.h"
#include "core/uuid.h"
#include "scene/component_defaults.h"
#include "scene/entity.h"

struct MeshComponent : ComponentDefaults {
  static constexpr std::string_view name = "Mesh";
  UUID mesh_handle;

  static void serialize(YAML::Emitter& out, const MeshComponent& c) {
    out << YAML::Key << "MeshComponent" << YAML::BeginMap;
    out << YAML::Key << "MeshHandle" << YAML::Value << c.mesh_handle;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["MeshComponent"];
    if (!n) return;
    auto& mc = entity.addComponent<MeshComponent>();
    mc.mesh_handle = n["MeshHandle"].as<UUID>();
  }
  static void drawUI(MeshComponent& c, AppContext& ctx) {
    ImGui::Text("Mesh");
    ImGui::SameLine();
    auto mesh_assets = ctx.assets.getAssetsMetadataOfType<Mesh>();
    auto current_name = mesh_assets.contains(c.mesh_handle) ? mesh_assets[c.mesh_handle].filepath.stem().c_str() : "";

    if (ImGui::BeginCombo("##mesh", current_name)) {
      for (const auto& [id, meta] : mesh_assets) {
        bool selected = id == c.mesh_handle;
        auto name = meta.filepath.stem().c_str();

        if (ImGui::Selectable(name, selected)) {
          c.mesh_handle = id;
        }
      }
      ImGui::EndCombo();
    }
  }
};