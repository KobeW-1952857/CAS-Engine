#include "editor/panels/asset_browser.h"

#include <ImGui/imgui.h>

#include <filesystem>

#include "Nexus/Log.h"
#include "core/asset_manager.h"
#include "core/project.h"
#include "utils/filesystem.h"

void AssetBrowser::drawDirectoryNode(const std::filesystem::path& path, SelectionContext& selection_context) {
  for (auto const& entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_directory()) {
      bool is_open = ImGui::TreeNode(entry.path().filename().c_str());
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("New material")) {
          AssetManager::createNewAsset<Material>(FileSystem::getProjectPath(entry.path()));
        }
        ImGui::EndPopup();
      }
      if (is_open) {
        drawDirectoryNode(entry.path(), selection_context);
        ImGui::TreePop();
      }
    } else {
      drawFileNode(entry.path(), selection_context);
    }
  }
}

void AssetBrowser::drawFileNode(const std::filesystem::path& path, SelectionContext& selection_context) {
  ImGui::PushID(path.c_str());

  if (ImGui::Selectable(std::format("{}", path.stem().string().c_str()).c_str(), m_selected == path)) {
    auto selected_path = FileSystem::getProjectPath(path);
    selection_context = AssetManager::getHandleFromPath(selected_path);
  }

  ImGui::PopID();
}

void AssetBrowser::onImGuiRender(SelectionContext& selection_context) {
  ImGui::Begin("Asset Browser");
  drawDirectoryNode(Project::getConfig().path, selection_context);

  ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;

  if (ImGui::BeginPopupContextWindow(nullptr, flags)) {
    if (ImGui::MenuItem("New material")) {
      AssetManager::createNewAsset<Material>();
    }
    ImGui::EndPopup();
  }
  ImGui::End();
}

void AssetBrowser::onUpdate(float dt) {
  // m_check_time -= dt;
  // if (m_check_time > 0.0f) return;
  // m_check_time = m_check_interval;
}
