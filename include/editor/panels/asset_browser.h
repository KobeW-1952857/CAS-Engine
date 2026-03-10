#pragma once

#include <filesystem>

#include "editor/selection_context.h"

class AssetBrowser {
 public:
  AssetBrowser() = default;
  AssetBrowser(const std::filesystem::path& base_path) : m_base_path(base_path) {}

  void onImGuiRender(SelectionContext& selection_context);
  void onUpdate(float dt);

 private:
  void drawDirectoryNode(const std::filesystem::path& path, SelectionContext& selection_context);
  void drawFileNode(const std::filesystem::path& path, SelectionContext& selection_context);

 private:
  const std::filesystem::path m_base_path = "assets";
  std::filesystem::path m_current_path = m_base_path;
  std::filesystem::path m_selected;

  //   std::unordered_map<std::filesystem::path, std::filesystem::directory_entry> m_cached_entries;
  //   const float m_check_interval = 1.0f;
  //   float m_check_time = 0.0f;
};