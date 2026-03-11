#include "editor/editor.h"

#include <Nexus/Log.h>
#include <imgui.h>

#include <cstdint>
#include <cstdlib>
#include <variant>

#include "core/asset_manager.h"
#include "core/project.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "utils/filesystem.h"
#include "utils/utils.h"

Editor::Editor() : m_viewport_size(1280, 720) { init(); }

Editor::Editor(const std::vector<std::string>& args) : m_viewport_size(1280, 720) {
  init();
  if (!args.empty()) {
    Project::load(args[0]);
    m_serializer.deserialize(Project::getConfig().start_scene);
  }
}

void Editor::init() {
  Renderer::init();
  FramebufferSpecification spec;
  spec.width = 1280;
  spec.height = 720;
  spec.attachments = {FramebufferTextureSpecification(FramebufferTextureFormat::RGBA8),             // Color
                      FramebufferTextureSpecification(FramebufferTextureFormat::RED_INTEGER),       // Entity ID
                      FramebufferTextureSpecification(FramebufferTextureFormat::DEPTH24STENCIL8)};  // Depth

  m_framebuffer = std::make_shared<Framebuffer>(spec);
  m_active_scene = std::make_shared<Scene>();
  m_serializer.setContext(m_active_scene);
  m_scene_hierarchy_panel.setContext(m_active_scene);

  Nexus::Logger::setCallback([](Nexus::LogLevel level, const std::string& message) {
    ConsoleMessage::Level uiLevel;
    switch (level) {
      case Nexus::LogLevel::Trace:
        uiLevel = ConsoleMessage::Level::Trace;
        break;
      case Nexus::LogLevel::Debug:
        uiLevel = ConsoleMessage::Level::Debug;
        break;
      case Nexus::LogLevel::Info:
        uiLevel = ConsoleMessage::Level::Info;
        break;
      case Nexus::LogLevel::Warn:
        uiLevel = ConsoleMessage::Level::Warn;
        break;
      case Nexus::LogLevel::Error:
        uiLevel = ConsoleMessage::Level::Error;
        break;
      case Nexus::LogLevel::Critical:
        uiLevel = ConsoleMessage::Level::Fatal;
        break;
      case Nexus::LogLevel::Off:
        break;
    }

    ConsolePanel::pushMessage(uiLevel, message);
  });
}

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
  m_framebuffer->resize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));

  m_framebuffer->clear({0.1f, 0.1f, 0.1f, 1.0f});
  m_framebuffer->clearAttachment(1, -1);

  if (m_active_scene) {
    Entity selected =
        std::holds_alternative<Entity>(m_selection_context) ? std::get<Entity>(m_selection_context) : Entity();
    m_active_scene->onRender(selected, m_editor_camera, m_viewport_size);
  }

  m_framebuffer->unbind();
}

void Editor::onImGuiRender() {
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

  if (Project::hasProject()) {
    static bool was_focused = true;
    bool is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
    if (is_focused && !was_focused) {
      AssetManager::syncFileSystem();
    }
    was_focused = is_focused;
    handleShortcuts();

    drawDockspace();
    drawViewport();
  } else {
    ImGui::Begin("Open/create Project");
    if (ImGui::Button("Open")) {
      Project::load();
      m_serializer.deserialize(FileSystem::resolvePath(Project::getConfig().start_scene));
    }
    ImGui::SameLine();
    if (ImGui::Button("New")) {
      Project::New();
    }
    ImGui::End();
  }
}

void Editor::drawDockspace() {
  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Open Project")) {
      Project::load();
      m_serializer.deserialize(FileSystem::resolvePath(Project::getConfig().start_scene));
    }
    if (ImGui::MenuItem("Save Project")) {
      Project::save();
      m_serializer.serialize(Project::getConfig().start_scene);
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  // drawDebugPanel();

  m_scene_hierarchy_panel.onImGuiRender(m_selection_context);
  m_asset_browser_panel.onImGuiRender(m_selection_context);
  m_properties_panel.onImGuiRender(m_selection_context);
  m_console_panel.onImGuiRender();
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

  uint32_t texture_id = m_framebuffer->getColorAttachmentRendererID();
  // NOLINTNEXTLINE
  ImGui::Image(reinterpret_cast<void*>(texture_id), viewport_panel_size, ImVec2(0, 1), ImVec2(1, 0));

  if (ImGui::IsMouseClicked(0) && m_viewport_hovered) {
    ImVec2 mouse_pos = ImGui::GetMousePos();
    ImVec2 viewport_min = ImGui::GetItemRectMin();

    mouse_pos.x -= viewport_min.x;
    mouse_pos.y -= viewport_min.y;

    // Flip Y coordinate for OpenGL
    mouse_pos.y = m_viewport_size.y - mouse_pos.y;

    int mouse_x = static_cast<int>(mouse_pos.x);
    int mouse_y = static_cast<int>(mouse_pos.y);

    int entity_id = m_framebuffer->readPixel(1, mouse_x, mouse_y);

    if (entity_id == -1) {
      m_selection_context = std::monostate{};
    } else {
      m_selection_context = Entity(static_cast<entt::entity>(entity_id), m_active_scene);
    }
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void Editor::handleShortcuts() {
  auto& io = ImGui::GetIO();
  if (io.WantTextInput) return;

  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
    Project::save();
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
    Project::load();
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
    Project::New();
  }
  if (ImGui::IsKeyPressed(ImGuiKey_F)) {
    // Focus on selected object
    if (std::holds_alternative<Entity>(m_selection_context)) {
      auto entity = std::get<Entity>(m_selection_context);
      if (entity) {
        auto tc = entity.getComponent<TransformComponent>();
        m_editor_camera.focusEntity(tc.Translation, glm::compMax(tc.Scale) * 5);
      }
    }
  }
}

void Editor::drawDebugPanel() {
  ImGui::Begin("Framebuffer Debug");

  ImVec2 size(256, 144);
  uint32_t debug_tex = m_framebuffer->getDebugEntityIDTextureID();
  ImGui::Text("Entity ID buffer");
  ImGui::Image(reinterpret_cast<void*>(debug_tex), size, ImVec2(0, 1), ImVec2(1, 0));

  // Legend
  ImGui::Separator();
  ImGui::Text("Legend");
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const float swatch_size = 16.0f;
  const float padding = 4.0f;

  for (const auto& [id, rgb] : m_framebuffer->getDebugEntityColorMap()) {
    auto [r, g, b] = rgb;
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Colored square
    draw_list->AddRectFilled(pos, ImVec2(pos.x + swatch_size, pos.y + swatch_size), IM_COL32(r, g, b, 255));
    draw_list->AddRect(
        pos, ImVec2(pos.x + swatch_size, pos.y + swatch_size), IM_COL32(255, 255, 255, 80)  // subtle border
    );

    // Label
    ImGui::SetCursorScreenPos(ImVec2(pos.x + swatch_size + padding, pos.y));
    if (id == -1) {
      ImGui::Text("Empty (-1)");
    } else {
      ImGui::Text("Entity ID: %d", id);
    }
  }

  ImGui::End();
}
