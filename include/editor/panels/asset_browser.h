#pragma once

#include <filesystem>

#include "core/app_context.h"
#include "editor/selection_context.h"

class AssetBrowser {
 public:
  explicit AssetBrowser(AppContext& context) : m_context(context) {}
  void onImGuiRender(SelectionContext& selection_context);
  void onUpdate(float dt);

 private:
  void drawDirectoryNode(const std::filesystem::path& path, SelectionContext& selection_context);
  void drawFileNode(const std::filesystem::path& path, SelectionContext& selection_context);

 private:
  AppContext& m_context;
  std::filesystem::path m_current_path;
  std::filesystem::path m_selected;
};