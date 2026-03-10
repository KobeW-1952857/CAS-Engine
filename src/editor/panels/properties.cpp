#include "editor/panels/properties.h"

#include <ImGui/imgui.h>
#include <ImGui/misc/cpp/imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>
#include <variant>

#include "core/asset_manager.h"
#include "core/uuid.h"
#include "editor/selection_context.h"
#include "scene/components.h"
#include "scene/entity.h"

static void drawMaterial(const std::shared_ptr<Material>& material, const AssetMetadata& meta_data) {
  auto shaders = AssetManager::getAssetsMetadataOfType<Shader>();
  auto& shader = material->shader;
  auto& current_meta = AssetManager::getAssetMetadata(shader->handle);

  ImGui::Text("Shader");
  ImGui::SameLine();
  if (ImGui::BeginCombo("##shader", current_meta.filepath.stem().c_str())) {
    for (const auto& [id, meta] : shaders) {
      bool selected = id == shader->handle;
      auto name = meta.filepath.filename().c_str();

      if (ImGui::Selectable(name, selected)) {
        material->shader = AssetManager::getAsset<Shader>(id);
        material->resetProperties();
      }
    }
    ImGui::EndCombo();
  }

  if (!shader) return;

  auto uniforms = shader->getActiveUniforms();
  for (auto uniform : uniforms) {
    if (uniform.name == "u_model" || uniform.name == "u_proj_view") continue;
    ImGui::Text("%s", uniform.name.c_str());
    ImGui::SameLine();
    switch (uniform.type) {
      case GL_INT: {
        auto val = material->getPropertyOrDefault(uniform.name, 0);
        if (ImGui::DragInt(std::format("##{}", uniform.name).c_str(), &val)) material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT: {
        auto val = material->getPropertyOrDefault(uniform.name, 0.0f);
        if (ImGui::DragFloat(std::format("##{}", uniform.name).c_str(), &val)) material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT_VEC2: {
        auto val = material->getPropertyOrDefault(uniform.name, glm::vec2(0.0f));
        if (ImGui::DragFloat2(std::format("##{}", uniform.name).c_str(), glm::value_ptr(val)))
          material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT_VEC3: {
        auto val = material->getPropertyOrDefault(uniform.name, glm::vec3(0.0f));
        if (ImGui::DragFloat3(std::format("##{}", uniform.name).c_str(), glm::value_ptr(val)))
          material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT_VEC4: {
        auto val = material->getPropertyOrDefault(uniform.name, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (ImGui::ColorEdit4(std::format("##{}", uniform.name).c_str(), glm::value_ptr(val)))
          material->setProperty(uniform.name, val);
        break;
      }
    }
  }
}

void PropertiesPanel::onImGuiRender(SelectionContext& selection_context) {
  ImGui::Begin("Properties");
  if (std::holds_alternative<Entity>(selection_context)) {
    drawComponents(std::get<Entity>(selection_context));
  } else if (std::holds_alternative<UUID>(selection_context)) {
    auto id = std::get<UUID>(selection_context);
    auto meta = AssetManager::getAssetMetadata(id);

    switch (meta.type) {
      case AssetType::Mesh: {
        auto asset = AssetManager::getAsset<Mesh>(id);
        break;
      }
      case AssetType::Material: {
        auto asset = AssetManager::getAsset<Material>(id);
        drawMaterial(asset, meta);
        break;
      }
      case AssetType::Shader: {
        auto asset = AssetManager::getAsset<Shader>(id);
        break;
      }
      default:
        break;
    }

  } else {
    ImGui::Text("Nothing selected");
  }
  ImGui::End();
}

template <typename T, typename UIFunction>
static void DrawComponent(Entity entity, UIFunction ui_function) {
  if (!entity.hasComponent<T>()) return;

  auto& component = entity.getComponent<T>();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
  ImGui::Separator();
  ImGui::PushID(typeid(T).hash_code());
  bool open = ImGui::TreeNodeEx("##Component", ImGuiTreeNodeFlags_DefaultOpen, "%s", T::name.c_str());
  ImGui::PopID();
  ImGui::PopStyleVar();

  if (open) {
    ui_function(component);
    ImGui::TreePop();
  }
}

void PropertiesPanel::drawComponents(Entity entity) {
  if (!entity) return;

  if (entity.hasComponent<TagComponent>()) {
    auto& tag = entity.getComponent<TagComponent>().Tag;
    if (ImGui::InputText("##Tag", &tag)) {
      entity.getComponent<TagComponent>().Tag = tag;
    }
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(-1);
  if (ImGui::Button("+")) ImGui::OpenPopup("Add component");

  if (ImGui::BeginPopup("Add component")) {
    displayAddComponent<MeshComponent>(entity);
    displayAddComponent<MaterialComponent>(entity);
    ImGui::EndPopup();
  }
  ImGui::PopItemWidth();

  DrawComponent<TransformComponent>(entity, [](auto& component) {
    ImGui::DragFloat3("Translation", glm::value_ptr(component.Translation));
    ImGui::DragFloat3("Rotation", glm::value_ptr(component.Rotation));
    ImGui::DragFloat3("Scale", glm::value_ptr(component.Scale));
  });

  DrawComponent<MeshComponent>(entity, [](MeshComponent& component) {
    ImGui::Text("Mesh");
    ImGui::SameLine();
    auto mesh_assets = AssetManager::getAssetsMetadataOfType<Mesh>();
    auto current_name =
        mesh_assets.contains(component.mesh_handle) ? mesh_assets[component.mesh_handle].filepath.stem().c_str() : "";

    if (ImGui::BeginCombo("##mesh", current_name)) {
      for (const auto& [id, meta] : mesh_assets) {
        bool selected = id == component.mesh_handle;
        auto name = meta.filepath.stem().c_str();

        if (ImGui::Selectable(name, selected)) {
          component.mesh_handle = id;
        }
      }
      ImGui::EndCombo();
    }
  });

  DrawComponent<MaterialComponent>(entity, [](MaterialComponent& component) {
    auto materials = AssetManager::getAssetsMetadataOfType<Material>();
    auto material = AssetManager::getAsset<Material>(component.material_handle);
    auto current_name = material ? AssetManager::getAssetMetadata(material->handle).filepath.stem().c_str() : "";

    if (ImGui::BeginCombo("##material", current_name)) {
      for (const auto& [id, meta] : materials) {
        bool selected = id == component.material_handle;
        auto name = meta.filepath.stem().c_str();

        if (ImGui::Selectable(name, selected)) {
          component.material_handle = id;
          material = AssetManager::getAsset<Material>(id);
        }
      }
      ImGui::EndCombo();
    }
  });
}

template <typename T>
void PropertiesPanel::displayAddComponent(Entity entity) {
  if (entity.hasComponent<T>()) return;

  if (ImGui::MenuItem(T::name.c_str())) {
    entity.addComponent<T>();
    ImGui::CloseCurrentPopup();
  }
}
