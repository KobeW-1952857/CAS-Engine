#pragma once

#include <filesystem>

#include "core/app_context.h"
#include "editor/selection_context.h"

class AssetBrowser {
 public:
  explicit AssetBrowser(AppContext& context) : m_context(context) {}
  void onImGuiRender(SelectionContext& selection_context);
  void onUpdate(float dt);

  void setDoubleClickCallback(std::function<void(UUID)> callback) { m_on_double_click = std::move(callback); }

 private:
  struct AssetContextMenu {
    bool can_create_folder = false;
    bool can_create_material = false;
    bool can_create_scene = false;
    bool can_delete = false;
    std::filesystem::path path;
    std::vector<std::pair<const char*, std::function<void()>>> custom_items;
  };
  void drawDirectoryNode(const std::filesystem::path& path, SelectionContext& selection_context);
  void drawFileNode(const std::filesystem::path& path, SelectionContext& selection_context);
  void drawContextMenu(const AssetContextMenu& menu);
  AssetContextMenu generateContextMenu(const std::filesystem::path& path);

 private:
  AppContext& m_context;
  std::filesystem::path m_current_path;
  std::filesystem::path m_selected;

  std::function<void(UUID)> m_on_double_click;
  std::unordered_map<std::filesystem::path, float> m_deleting;
};