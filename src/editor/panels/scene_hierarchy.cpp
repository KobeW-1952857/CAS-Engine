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
    for (auto& [id, entity] : m_context->m_entity_map) {
      if (!entity.hasComponent<ParentComponent>()) drawEntityNode(entity, selection_context, entity_to_delete);
    }
    if (entity_to_delete) m_context->destroyEntity(entity_to_delete);

    if (ImGui::Button("Create entity", ImVec2(-1.0f, 0.0f))) m_context->createEntity();

    ImGui::InvisibleButton("##scene_drop_target", ImVec2(-1.0f, -1.0f));
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
        UUID dragged_id = *static_cast<const UUID*>(payload->Data);
        Entity dragged = m_context->getEntity(dragged_id);
        if (dragged) m_context->unParent(dragged);
      }
      ImGui::EndDragDropTarget();
    }
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
  if (!entity.hasComponent<ChildrenComponent>()) flags |= ImGuiTreeNodeFlags_Leaf;

  ImGui::PushID(static_cast<uint32_t>(entity));
  bool opened = ImGui::TreeNodeEx("##EntityNode", flags, "%s", tag.c_str());

  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover)) {
    UUID id = entity.getComponent<IDComponent>().ID;
    ImGui::SetDragDropPayload("Entity", &id, sizeof(UUID));
    ImGui::Text("%s", tag.c_str());
    ImGui::EndDragDropSource();
  }

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
      UUID drag_id = *static_cast<UUID*>(payload->Data);
      Entity drag_entity = m_context->getEntity(drag_id);

      if (drag_entity && drag_entity != entity && !isDescendantOf(drag_entity, entity))
        m_context->setParent(drag_entity, entity);
    }
    ImGui::EndDragDropTarget();
  }

  ImGui::PopID();

  if (ImGui::IsItemClicked()) selection_context = entity;

  bool entity_deleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete entity")) entity_deleted = true;
    ImGui::EndPopup();
  }

  if (opened) {
    if (entity.hasComponent<ChildrenComponent>()) {
      for (const auto& child_id : entity.getComponent<ChildrenComponent>().children) {
        Entity child = m_context->getEntity(child_id);
        if (child) drawEntityNode(child, selection_context, entity_to_delete);
      }
    }
    ImGui::TreePop();
  }

  if (entity_deleted) {
    entity_to_delete = entity;
    if (is_selected) selection_context = std::monostate{};
  }
}

bool SceneHierarchy::isDescendantOf(Entity potential_ancestor, Entity entity) const {
  if (!entity) return false;
  if (entity == potential_ancestor) return true;

  if (!entity.hasComponent<ChildrenComponent>()) return false;

  for (UUID child_id : entity.getComponent<ChildrenComponent>().children) {
    Entity child = m_context->getEntity(child_id);
    if (!child) continue;

    UUID child_uuid = child.getComponent<IDComponent>().ID;
    UUID ancestor_uuid = potential_ancestor.getComponent<IDComponent>().ID;

    if (child_uuid == ancestor_uuid) return true;
    if (isDescendantOf(potential_ancestor, child)) return true;
  }
  return false;
}
