#pragma once

#include <memory>

#include "editor/editor_camera.h"
#include "editor/scene_hierarchy.h"
#include "renderer/framebuffer.h"
#include "scene/scene.h"
#include "scene/scene_serializer.h"

class Editor {
 public:
  Editor();
  ~Editor() = default;

  void init();
  void setContext(const std::shared_ptr<Scene>& scene);
  void onUpdate(float dt);
  void onImGuiRender();

 private:
  void drawDockspace();
  void drawViewport();

 private:
  std::shared_ptr<Scene> m_active_scene;
  SceneSerializer m_serializer;
  EditorCamera m_editor_camera;

  SceneHierarchy m_scene_hierarchy_panel;

  std::shared_ptr<Framebuffer> m_framebuffer;

  glm::vec2 m_viewport_size{0.0f, 0.0f};
  bool m_viewport_focused = false;
  bool m_viewport_hovered = false;
};