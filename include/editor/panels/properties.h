#pragma once

#include "editor/selection_context.h"

class PropertiesPanel {
 public:
  PropertiesPanel() = default;

  void onImGuiRender(SelectionContext& selection_context);

 private:
  void drawComponents(Entity entity);

  template <typename T>
  void displayAddComponent(Entity entity);
};