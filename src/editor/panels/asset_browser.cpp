#include "editor/panels/asset_browser.h"

#include <ImGui/imgui.h>

#include <filesystem>

#include "core/asset_manager.h"
#include "core/project.h"
#include "utils/filesystem.h"

void AssetBrowser::drawDirectoryNode(const std::filesystem::path& path, SelectionContext& selection_context) {
  auto& assets = m_context.assets;
  auto& fs = m_context.filesystem;

  for (auto const& entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_directory()) {
      bool is_open = ImGui::TreeNode(entry.path().filename().c_str());
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("New material")) {
          assets.createNewAsset<Material>(fs.getProjectPath(entry.path()));
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
  if (path.extension() == ".casreg" || path.extension() == ".cproj") return;
  ImGui::PushID(path.c_str());

  auto selected_path = m_context.filesystem.getProjectPath(path);
  auto handle = m_context.assets.getHandleFromPath(selected_path);

  if (ImGui::Selectable(std::format("{}", path.stem().string().c_str()).c_str(), m_selected == path)) {
    selection_context = handle;
  }

  if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && m_on_double_click) {
    m_on_double_click(handle);
  }

  ImGui::PopID();
}

void AssetBrowser::onImGuiRender(SelectionContext& selection_context) {
  ImGui::Begin("Asset Browser");
  drawDirectoryNode(m_context.project.getConfig().path, selection_context);

  ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;

  if (ImGui::BeginPopupContextWindow(nullptr, flags)) {
    if (ImGui::MenuItem("New material")) {
      m_context.assets.createNewAsset<Material>();
    }
    ImGui::EndPopup();
  }
  ImGui::End();
}

void AssetBrowser::onUpdate(float dt) {}
