#include "editor/panels/scene_hierarchy.h"

#include <ImGui/imgui.h>
#include <ImGui/misc/cpp/imgui_stdlib.h>
#include <Nexus/Log.h>

#include <cstdint>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <variant>

#include "scene/components.h"
#include "scene/entity.h"

void SceneHierarchy::onImGuiRender(SelectionContext& selection_context) {
  ImGui::Begin("Scene Hierarchy");
  if (m_context) {
    Entity entity_to_delete{};
    for (auto& entity : m_context->m_entity_map) {
      drawEntityNode(entity.second, selection_context, entity_to_delete);
    }
    if (entity_to_delete) {
      m_context->destroyEntity(entity_to_delete);
    }

    if (ImGui::Button("Create entity", ImVec2(-1.0f, 0.0f))) m_context->createEntity();
  }
  ImGui::End();
}

void SceneHierarchy::setContext(const std::shared_ptr<Scene>& context) { m_context = context; }

void SceneHierarchy::drawEntityNode(Entity entity, SelectionContext& selection_context, Entity& entity_to_delete) {
  if (!entity) return;

  auto& tag = entity.getComponent<TagComponent>().tag;

  bool is_selected = false;
  if (const Entity* selected = std::get_if<Entity>(&selection_context)) {
    is_selected = (*selected == entity);
  }

  ImGuiTreeNodeFlags flags = (is_selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGui::PushID(static_cast<uint32_t>(entity));
  bool opened = ImGui::TreeNodeEx("##EntityNode", flags, "%s", tag.c_str());
  ImGui::PopID();

  if (ImGui::IsItemClicked()) selection_context = entity;

  bool entity_deleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete entity")) entity_deleted = true;
    ImGui::EndPopup();
  }

  if (opened) {
    // TODO(Kobe): draw child nodes
    ImGui::TreePop();
  }

  if (entity_deleted) {
    entity_to_delete = entity;
    if (is_selected) selection_context = std::monostate{};
  }
}
