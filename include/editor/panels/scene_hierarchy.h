#pragma once

#include <memory>

#include "editor/selection_context.h"
#include "scene/entity.h"
#include "scene/scene.h"

class SceneHierarchy {
 public:
  SceneHierarchy() = default;
  SceneHierarchy(const std::shared_ptr<Scene>& context) : m_context(context) {}

  void onImGuiRender(SelectionContext& selection_context);

  void setContext(const std::shared_ptr<Scene>& context);

 private:
  void drawEntityNode(Entity entity, SelectionContext& selection_context, Entity& entity_to_delete);
  void drawComponents(Entity entity);

  template <typename T>
  void displayAddComponent();

 private:
  std::shared_ptr<Scene> m_context;
};