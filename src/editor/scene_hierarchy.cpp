#include "editor/scene_hierarchy.h"

#include <ImGui/imgui.h>
#include <ImGui/misc/cpp/imgui_stdlib.h>
#include <Nexus/Log.h>

#include <cstdint>
#include <glm/gtc/type_ptr.hpp>

#include "core/asset_manager.h"
#include "scene/components.h"

void SceneHierarchy::setContext(const std::shared_ptr<Scene>& context) {
  m_context = context;
  m_selection_context = {};
}

void SceneHierarchy::setSelectionContext(Entity entity) { m_selection_context = entity; }

void SceneHierarchy::onImGuiRender() {
  ImGui::Begin("Scene Hierarchy");
  if (m_context) {
    for (auto& entity : m_context->m_entity_map) {
      drawEntityNode(entity.second);
    }

    if (ImGui::Button("Create entity", ImVec2(-1.0f, 0.0f))) m_context->createEntity();
  }
  ImGui::End();

  ImGui::Begin("Properties");
  if (m_selection_context) {
    drawComponents(m_selection_context);
  }
  ImGui::End();
}

void SceneHierarchy::drawEntityNode(Entity entity) {
  if (!entity) return;

  auto& tag = entity.getComponent<TagComponent>().Tag;

  ImGuiTreeNodeFlags flags = ((m_selection_context == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                             ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
  bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());
  if (ImGui::IsItemClicked()) {
    setSelectionContext(entity);
  }

  bool entity_deleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete entity")) entity_deleted = true;
    ImGui::EndPopup();
  }

  if (opened) {
    // TODO: draw child nodes
    ImGui::TreePop();
  }

  if (entity_deleted) {
    m_context->m_entity_map.erase(entity.getComponent<IDComponent>().ID);
    m_selection_context = {};
  }
}

template <typename T, typename UIFunction>
static void DrawComponent(Entity entity, UIFunction ui_function) {
  if (!entity.hasComponent<T>()) return;

  auto& component = entity.getComponent<T>();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
  ImGui::Separator();
  bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "%s", T::name.c_str());
  ImGui::PopStyleVar();

  if (open) {
    ui_function(component);
    ImGui::TreePop();
  }
}

void SceneHierarchy::drawComponents(Entity entity) {
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
    displayAddComponent<MeshComponent>();
    displayAddComponent<MaterialComponent>();
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
    auto current_name = mesh_assets.contains(component.mesh_handle)
                            ? mesh_assets[component.mesh_handle].filepath.filename().c_str()
                            : "";

    if (ImGui::BeginCombo("##mesh", current_name)) {
      for (auto [id, meta] : mesh_assets) {
        bool selected = id == component.mesh_handle;
        auto name = meta.filepath.filename().c_str();

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
    auto current_name = material ? AssetManager::getAssetMetadata(material->handle).filepath.filename().c_str() : "";

    if (ImGui::BeginCombo("##material", current_name)) {
      for (auto [id, meta] : materials) {
        bool selected = id == component.material_handle;
        auto name = meta.filepath.filename().c_str();

        if (ImGui::Selectable(name, selected)) {
          component.material_handle = id;
          material = AssetManager::getAsset<Material>(id);
        }
      }
      ImGui::EndCombo();
    }
    if (!material) return;

    auto shaders = AssetManager::getAssetsMetadataOfType<Shader>();
    auto& shader = material->shader;
    current_name = shader ? AssetManager::getAssetMetadata(shader->handle).filepath.filename().c_str() : "";

    ImGui::Text("Shader");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##shader", current_name)) {
      for (auto [id, meta] : shaders) {
        bool selected = id == shader->handle;
        auto name = meta.filepath.filename().c_str();

        if (ImGui::Selectable(name, selected)) shader = AssetManager::getAsset<Shader>(id);
      }
      ImGui::EndCombo();
    }

    if (!shader) return;

    ImGui::Text("Material properties");
    auto uniforms = shader->getActiveUniforms();
    for (auto uniform : uniforms) {
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
          if (ImGui::DragFloat(std::format("##{}", uniform.name).c_str(), &val))
            material->setProperty(uniform.name, val);
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
        case GL_FLOAT_MAT4: {
          auto val = material->getPropertyOrDefault(uniform.name, glm::mat4(1.0f));
          ImGui::Text("Temporary, this shows a matrix");
          break;
        }
        default: {
          ImGui::Text("unknown uniform type");
          break;
        }
      }
    }
  });
}

template <typename T>
void SceneHierarchy::displayAddComponent() {
  if (m_selection_context.hasComponent<T>()) return;

  if (ImGui::MenuItem(T::name.c_str())) {
    m_selection_context.addComponent<T>();
    ImGui::CloseCurrentPopup();
  }
}