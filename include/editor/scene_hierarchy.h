#pragma once

#include <memory>

#include "scene/entity.h"
#include "scene/scene.h"

class SceneHierarchy {
 public:
  SceneHierarchy() = default;
  SceneHierarchy(const std::shared_ptr<Scene>& context) : m_context(context) {}

  void onImGuiRender();

  void setContext(const std::shared_ptr<Scene>& context);

  void setSelectionContext(Entity entity);
  Entity getSelectionContext() { return m_selection_context; }

 private:
  void drawEntityNode(Entity entity);
  void drawComponents(Entity entity);

  template <typename T>
  void displayAddComponent();

 private:
  std::shared_ptr<Scene> m_context;
  Entity m_selection_context{};
};