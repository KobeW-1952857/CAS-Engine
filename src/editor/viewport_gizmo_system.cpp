#include "editor/viewport_gizmo_system.h"

#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include "scene/components.h"

void ViewportGizmoSystem::onImGuiRender(Entity entity, Scene& scene, const EditorCamera& camera, int gizmo_type) {
  if (!entity) return;

  // Reset sub-selection state if the user clicks a different entity
  if (m_last_entity != entity) {
    m_active_control_point = -1;
    m_last_entity = entity;
  }

  // Dispatch to individual component gizmos
  drawTransformGizmo(entity, scene, camera, gizmo_type);
  drawHandleGizmos(entity, scene, camera);
}

void ViewportGizmoSystem::drawTransformGizmo(Entity entity, Scene& scene, const EditorCamera& camera, int gizmo_type) {
  if (!entity.hasComponent<TransformComponent>()) return;

  auto& tc = entity.getComponent<TransformComponent>();
  glm::mat4 world_transform = tc.getWorldTransform(scene, entity);
  glm::mat4 delta(1.0f);

  ImGuizmo::Manipulate(glm::value_ptr(camera.getViewMatrix()), glm::value_ptr(camera.getProjectionMatrix()),
                       static_cast<ImGuizmo::OPERATION>(gizmo_type), ImGuizmo::WORLD, glm::value_ptr(world_transform),
                       glm::value_ptr(delta));

  if (ImGuizmo::IsUsing()) {
    glm::mat4 parent_transform = glm::mat4(1.0f);
    if (entity.hasComponent<ParentComponent>()) {
      Entity parent = scene.getEntity(entity.getComponent<ParentComponent>().parent_id);
      if (parent) parent_transform = parent.getComponent<TransformComponent>().getWorldTransform(scene, parent);
    }
    glm::mat4 local_transform = glm::inverse(parent_transform) * world_transform;
    glm::vec3 translation, rotation, scale;
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(local_transform), glm::value_ptr(translation),
                                          glm::value_ptr(rotation), glm::value_ptr(scale));
    tc.translation = translation;
    tc.rotation = glm::radians(rotation);
    tc.scale = scale;
  }
}

void ViewportGizmoSystem::drawHandleGizmos(Entity entity, Scene& scene, const EditorCamera& camera) {
  auto& tc = entity.getComponent<TransformComponent>();

  if (auto* bc = entity.tryGetComponent<BezierComponent>()) {
    glm::mat4 world_transform = tc.getWorldTransform(scene, entity);
    int i = 0;
    for (auto& point : bc->control_points) {
      glm::vec3 world_pos = world_transform * glm::vec4(point, 1.0f);
      if (ImGuizmo::IsOver(glm::value_ptr(world_pos), 10.0f)) {
        // Hover feedback of some sort
        if (ImGui::IsMouseClicked(0)) {
          m_active_control_point = i;
        }
      }
      i++;
    }

    if (m_active_control_point != -1 && m_active_control_point < bc->control_points.size()) {
      glm::vec3 active_pt = bc->control_points[m_active_control_point];
      glm::mat4 pt_transform = glm::translate(glm::mat4(1.0f), active_pt);
      glm::mat4 delta(1.0f);
      ImGuizmo::PushID(m_active_control_point);
      ImGuizmo::Manipulate(glm::value_ptr(camera.getViewMatrix()), glm::value_ptr(camera.getProjectionMatrix()),
                           ImGuizmo::TRANSLATE, ImGuizmo::WORLD, glm::value_ptr(pt_transform), glm::value_ptr(delta));
      if (ImGuizmo::IsUsing()) {
        active_pt = delta[3];
        bc->control_points[m_active_control_point] += active_pt;
      }
      ImGuizmo::PopID();
    }
  }
}