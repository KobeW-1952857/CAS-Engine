#pragma once

#include <string_view>

#include "core/app_context.h"
#include "core/uuid.h"
#include "scene/component_defaults.h"
#include "scene/entity.h"

struct MaterialComponent : ComponentDefaults {
  static constexpr std::string_view name = "Material";
  UUID material_handle = 2;
  static void serialize(YAML::Emitter& out, const MaterialComponent& c) {
    out << YAML::Key << "MaterialComponent" << YAML::BeginMap;
    out << YAML::Key << "MaterialHandle" << YAML::Value << c.material_handle;
    out << YAML::EndMap;
  }
  static void deserialize(const YAML::Node& node, Entity entity) {
    auto n = node["MaterialComponent"];
    if (n) {
      auto& mc = entity.addComponent<MaterialComponent>();
      mc.material_handle = n["MaterialHandle"].as<UUID>();
    }
  }
  static void drawUI(MaterialComponent& c, AppContext& ctx) {
    auto materials = ctx.assets.getAssetsMetadataOfType<Material>();
    auto material = ctx.assets.getAsset<Material>(c.material_handle);
    auto current_name = material ? ctx.assets.getAssetMetadata(material->handle).filepath.stem().c_str() : "";

    if (ImGui::BeginCombo("##material", current_name)) {
      for (const auto& [id, meta] : materials) {
        bool selected = id == c.material_handle;
        auto name = meta.filepath.stem().c_str();

        if (ImGui::Selectable(name, selected)) {
          c.material_handle = id;
          material = ctx.assets.getAsset<Material>(id);
        }
      }
      ImGui::EndCombo();
    }
  }
};