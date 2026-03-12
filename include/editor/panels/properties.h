#pragma once

#include "core/app_context.h"
#include "editor/selection_context.h"

class PropertiesPanel {
 public:
  explicit PropertiesPanel(AppContext& context) : m_context(context) {}

  void onImGuiRender(SelectionContext& selection_context);

 private:
  void drawComponents(Entity entity);
  void drawMaterial(const std::shared_ptr<Material>& material, AssetMetadata& meta_data);

  template <typename T>
  void displayAddComponent(Entity entity);

 private:
  AppContext& m_context;
};