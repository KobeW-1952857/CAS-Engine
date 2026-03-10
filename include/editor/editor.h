#pragma once

#include <memory>

#include "editor/editor_camera.h"
#include "editor/panels/asset_browser.h"
#include "editor/panels/console.h"
#include "editor/panels/properties.h"
#include "editor/panels/scene_hierarchy.h"
#include "editor/selection_context.h"
#include "renderer/framebuffer.h"
#include "scene/scene.h"
#include "scene/scene_serializer.h"

class Editor {
 public:
  Editor();
  Editor(const std::vector<std::string>& args);
  ~Editor() = default;

  void setContext(const std::shared_ptr<Scene>& scene);
  void onUpdate(float dt);
  void onImGuiRender();

 private:
  void drawDockspace();
  void drawViewport();
  void init();
  void handleShortcuts();

 private:
  std::shared_ptr<Scene> m_active_scene;
  SceneSerializer m_serializer;
  EditorCamera m_editor_camera;

  SelectionContext m_selection_context;
  SceneHierarchy m_scene_hierarchy_panel;
  AssetBrowser m_asset_browser_panel;
  ConsolePanel m_console_panel;
  PropertiesPanel m_properties_panel;

  std::shared_ptr<Framebuffer> m_framebuffer;

  glm::vec2 m_viewport_size{0.0f, 0.0f};
  bool m_viewport_focused = false;
  bool m_viewport_hovered = false;
};