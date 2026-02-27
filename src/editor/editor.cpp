#include "editor/editor.h"

#include <imgui.h>

#include <cstdint>

#include "utils/file_dialog.h"
#include "utils/utils.h"

Editor::Editor() : m_viewport_size(1280, 720) { m_framebuffer = std::make_shared<Framebuffer>(1280, 720); }

void Editor::init() {}

void Editor::setContext(const std::shared_ptr<Scene>& scene) {
  m_active_scene = scene;
  m_scene_hierarchy_panel.setContext(scene);
  m_serializer.setContext(scene);
}

void Editor::onUpdate(float dt) {
  if (m_viewport_focused || m_viewport_hovered) {
    m_editor_camera.onUpdate(dt);
  }

  m_framebuffer->bind();
  m_framebuffer->resize(m_viewport_size.x, m_viewport_size.y);
  m_framebuffer->clear({0.1f, 0.1f, 0.1f, 1.0f});

  if (m_active_scene) {
    glm::mat4 view_proj = m_editor_camera.getViewProjectionMatrix();
    m_active_scene->onRender(view_proj);
  }

  m_framebuffer->unbind();
}

void Editor::onImGuiRender() {
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

  drawDockspace();
  drawViewport();
}

void Editor::drawDockspace() {
  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Open scene")) {
      m_serializer.deserialize(FileDialog::openFile());
    }
    if (ImGui::MenuItem("Save scene")) {
      m_serializer.serialize(FileDialog::saveFile());
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  m_scene_hierarchy_panel.onImGuiRender();
}

void Editor::drawViewport() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Viewport");
  m_viewport_focused = ImGui::IsWindowFocused();
  m_viewport_hovered = ImGui::IsWindowHovered();

  ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
  if (m_viewport_size.x != viewport_panel_size.x || m_viewport_size.y != viewport_panel_size.y) {
    m_viewport_size = {viewport_panel_size.x, viewport_panel_size.y};
    m_editor_camera.setViewportSize(m_viewport_size.x, m_viewport_size.y);
  }

  uint32_t texture_id = m_framebuffer->getTexture();
  ImGui::Image(reinterpret_cast<void*>(texture_id), viewport_panel_size, ImVec2(0, 1), ImVec2(1, 0));

  ImGui::End();
  ImGui::PopStyleVar();
}
