#pragma once

#include "editor/editor_camera.h"
#include "scene/entity.h"
#include "scene/scene.h"

class ViewportGizmoSystem {
 public:
  // Dispatches rendering for the currently selected entity
  void onImGuiRender(Entity entity, Scene& scene, const EditorCamera& camera, int gizmo_type);

 private:
  void drawTransformGizmo(Entity entity, Scene& scene, const EditorCamera& camera, int gizmo_type);
  void drawHandleGizmos(Entity entity, Scene& scene, const EditorCamera& camera);

 private:
  // Store editor-session state here
  Entity m_last_entity{};
  int m_active_control_point = -1;
};