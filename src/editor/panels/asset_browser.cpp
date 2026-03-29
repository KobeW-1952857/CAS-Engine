#include "editor/panels/asset_browser.h"

#include <ImGui/imgui.h>

#include <filesystem>

#include "core/asset_manager.h"
#include "core/project.h"
#include "utils/IconBindings.h"
#include "utils/filesystem.h"

void AssetBrowser::drawContextMenu(const AssetContextMenu& menu) {
  if (!ImGui::BeginPopupContextItem()) return;

  if (menu.can_create_folder)
    if (ImGui::MenuItem(ICON_MD_FOLDER " New Folder")) m_context.filesystem.createDirectory(menu.path / "New Folder");

  if (menu.can_create_material)
    if (ImGui::MenuItem(ICON_MD_PALETTE " New Material"))
      m_context.assets.createNewAsset<Material>(m_context.filesystem.getProjectPath(menu.path));

  if (menu.can_create_scene)
    if (ImGui::MenuItem(ICON_MD_IMAGE_SIZE_SELECT_LARGE " New Scene"))
      m_context.assets.createNewAsset<Scene>(m_context.filesystem.getProjectPath(menu.path));

  if (menu.can_create_folder || menu.can_create_material || menu.can_create_scene) ImGui::Separator();
  if (!menu.custom_items.empty()) {
    for (const auto& [label, action] : menu.custom_items)
      if (ImGui::MenuItem(label)) action();
    ImGui::Separator();
  }

  if (menu.can_delete) {
    if (ImGui::MenuItem(ICON_MD_DELETE " Delete")) m_context.filesystem.deletePath(menu.path);
  }

  ImGui::EndPopup();
}

AssetBrowser::AssetContextMenu AssetBrowser::generateContextMenu(const std::filesystem::path& path) {
  AssetBrowser::AssetContextMenu menu;
  menu.path = path;

  if (std::filesystem::is_directory(path)) {
    menu.can_create_folder = true;
    menu.can_create_material = true;
    menu.can_create_scene = true;
    menu.can_delete = !std::filesystem::is_empty(path);
  } else {
    auto selected_path = m_context.filesystem.getProjectPath(path);
    auto handle = m_context.assets.getHandleFromPath(selected_path);
    auto meta = m_context.assets.getAssetMetadata(handle);
    if (meta.type == AssetType::Material) {
      menu.can_delete = true;
    } else if (meta.type == AssetType::Scene) {
      menu.can_delete = true;
    }
    switch (meta.type) {
      case AssetType::Scene:
        if (handle != m_context.project.getDefaultScene())
          menu.custom_items.push_back(
              {"Set as Default", [this, handle] { m_context.project.setDefaultScene(handle); }});
        break;
      default:
        break;
    }
  }

  return menu;
}

void AssetBrowser::drawDirectoryNode(const std::filesystem::path& path, SelectionContext& selection_context) {
  auto& assets = m_context.assets;
  auto& fs = m_context.filesystem;
  // collect entries
  std::vector<std::filesystem::directory_entry> entries(std::filesystem::directory_iterator(path),
                                                        std::filesystem::directory_iterator{});

  // sort: directories first, then files, alphabetically within each group
  std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
    if (a.is_directory() != b.is_directory()) return a.is_directory() > b.is_directory();  // dirs first
    return a.path().filename() < b.path().filename();                                      // then alphabetical
  });

  for (auto const& entry : entries) {
    if (entry.is_directory()) {
      ImGuiID id = ImGui::GetID(entry.path().c_str());
      bool is_open = ImGui::GetStateStorage()->GetBool(id, false);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

      is_open = ImGui::TreeNode(entry.path().c_str(), "%s %s", is_open ? ICON_MD_FOLDER_OPEN : ICON_MD_FOLDER,
                                entry.path().filename().c_str());
      ImGui::PopStyleVar();

      auto menu = generateContextMenu(entry.path());
      drawContextMenu(menu);
      if (is_open) {
        drawDirectoryNode(entry.path(), selection_context);
        ImGui::TreePop();
      }
    } else {
      drawFileNode(entry.path(), selection_context);
    }
  }
}

constexpr const char* getAssetTypeIcon(AssetType type) {
  switch (type) {
    case AssetType::Scene:
      return ICON_MD_IMAGE_SIZE_SELECT_LARGE;
    case AssetType::Mesh:
      return ICON_MD_SHAPE;
    case AssetType::Material:
      return ICON_MD_PALETTE;
    default:
      return ICON_MD_FILE;
  }
}

void AssetBrowser::drawFileNode(const std::filesystem::path& path, SelectionContext& selection_context) {
  if (path.extension() == ".casreg" || path.extension() == ".cproj") return;
  ImGui::PushID(path.c_str());

  auto selected_path = m_context.filesystem.getProjectPath(path);
  auto handle = m_context.assets.getHandleFromPath(selected_path);
  auto meta = m_context.assets.getAssetMetadata(handle);

  if (meta.type == AssetType::Scene && handle == m_context.project.getDefaultScene()) {
    // Highlight the default scene in a different color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
  }

  if (ImGui::Selectable(std::format("{} {}", getAssetTypeIcon(meta.type), path.stem().string().c_str()).c_str(),
                        m_selected == path)) {
    selection_context = handle;
  }
  if (meta.type == AssetType::Scene && handle == m_context.project.getDefaultScene()) ImGui::PopStyleColor();

  auto menu = generateContextMenu(path);
  drawContextMenu(menu);

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
