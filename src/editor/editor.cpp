#include "editor/editor.h"

#include <IconsFontAwesome7.h>
#include <Nexus/Log.h>
#include <imgui.h>

#include <cstdint>
#include <cstdlib>
#include <glm/gtx/string_cast.hpp>
#include <variant>

#include "core/asset_manager.h"
#include "core/project.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer/systems/bezier_renderer_system.h"
#include "renderer/systems/line_renderer_system.h"
#include "renderer/systems/mesh_renderer_system.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scene/systems/line_follower_system.h"
#include "utils/utils.h"

Editor::Editor() : m_viewport_size(1280, 720), m_asset_browser_panel(m_context), m_properties_panel(m_context) {
  init();
}

Editor::Editor(const std::vector<std::string>& args)
    : m_viewport_size(1280, 720), m_asset_browser_panel(m_context), m_properties_panel(m_context) {
  init();
  if (!args.empty()) {
    m_context.project.load(args[0]);
    openScene(m_context.project.getConfig().start_scene);
  }
}

std::shared_ptr<Scene> Editor::makeScene() { return std::make_shared<Scene>(&m_context.assets, &m_context.renderer); }

void Editor::openScene(UUID handle) {
  auto scene = m_context.assets.getAsset<Scene>(handle);
  if (!scene) return;
  setContext(scene);

  scene->registerRenderSystem(std::make_unique<MeshRenderSystem>());
  scene->registerRenderSystem(std::make_unique<LineRendererSystem>(m_context.filesystem));
  scene->registerRenderSystem(std::make_unique<BezierRendererSystem>(m_context.filesystem));

  scene->registerLogicSystem(std::make_unique<LineFollowerSystem>());
}

static void setTheme();

void Editor::init() {
  setTheme();
  ImGui::GetStyle().WindowRounding = 12.0f;
  ImGui::GetStyle().FrameRounding = 12.0f;
  ImGui::GetStyle().GrabRounding = 8.0f;
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF(
      m_context.filesystem.resolvePath("engine://fonts/Comfortaa-VariableFont_wght.ttf").c_str(), 16.0f);
  ImFontConfig config;
  config.MergeMode = true;
  config.GlyphMinAdvanceX = 13.0f;
  config.GlyphOffset.y = 1.5f;
  config.GlyphOffset.x = 1.0f;
  config.PixelSnapH = true;
  static const ImWchar icon_ranges[] = {0xf000, 0xf8ff, 0};
  io.Fonts->AddFontFromFileTTF(m_context.filesystem.resolvePath("engine://fonts/fa-solid-900.otf").c_str(), 13.0f,
                               &config, icon_ranges);
  io.Fonts->Build();

  m_context.renderer.init(m_context.filesystem);
  m_context.assets.setRenderer(&m_context.renderer);

  FramebufferSpecification spec;
  spec.width = 1280;
  spec.height = 720;
  spec.attachments = {FramebufferTextureFormat::RGBA8,             // Color
                      FramebufferTextureFormat::RED_INTEGER,       // Entity ID
                      FramebufferTextureFormat::DEPTH24STENCIL8};  // Depth

  m_framebuffer = std::make_shared<Framebuffer>(spec);
  if (m_context.project.getConfig().start_scene) {
    openScene(m_context.project.getConfig().start_scene);
  } else {
    m_active_scene = makeScene();
  }

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

  m_asset_browser_panel.setDoubleClickCallback([this](UUID handle) {
    auto& meta = m_context.assets.getAssetMetadata(handle);
    switch (meta.type) {
      case AssetType::Scene: {
        openScene(handle);
        break;
      }
      default:
        break;
    }
  });

  m_grid_overlay.emplace(m_context.filesystem);
}

void Editor::setContext(const std::shared_ptr<Scene>& scene) {
  m_active_scene = scene;
  m_scene_hierarchy_panel.setContext(scene);
  m_context.active_scene = scene.get();
}

void Editor::onUpdate(float dt) {
  m_context.clock.advanceFrame(dt);

  if (m_viewport_focused || m_viewport_hovered) m_editor_camera.onUpdate(dt, !ImGui::GetIO().WantCaptureKeyboard);

  if (m_active_scene) {
    for (uint64_t i = 0; i < m_context.clock.pendingTicks(); ++i) m_active_scene->onUpdate(m_context.clock.tickDt());
  }

  m_framebuffer->resize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));

  m_framebuffer->bind();
  m_framebuffer->clear({0.1f, 0.1f, 0.1f, 1.0f});
  m_framebuffer->clearAttachment(1, -1);

  if (m_active_scene) {
    Entity selected =
        std::holds_alternative<Entity>(m_selection_context) ? std::get<Entity>(m_selection_context) : Entity();
    m_active_scene->onRender(m_editor_camera.toCameraData(), selected, [this] {
      if (!m_grid_overlay || !m_grid_overlay->enabled) return;
      GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE};
      glDrawBuffers(2, buffers);

      if (m_grid_overlay) m_grid_overlay->render();

      // Restore so the framebuffer is in a clean state after unbind.
      GLenum restore[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
      glDrawBuffers(2, restore);
    });
  }

  m_framebuffer->unbind();
}

void Editor::onImGuiRender() {
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

  if (m_context.project.hasProject()) {
    static bool was_focused = true;
    bool is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
    if (is_focused && !was_focused) {
      m_context.assets.syncFileSystem();
    }
    was_focused = is_focused;
    handleShortcuts();

    drawDockspace();
    drawViewport();
  } else {
    ImGui::Begin("Open/create Project");
    if (ImGui::Button("Open")) {
      m_context.project.load();
      openScene(m_context.project.getConfig().start_scene);
    }
    ImGui::SameLine();
    if (ImGui::Button("New")) {
      m_context.project.New();
    }
    ImGui::End();
  }
}

void DrawMainStatusBar() {
  // 1. Get the Main Viewport (the whole GLFW window)
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  // 2. Define the height of your bar
  float footerHeight = ImGui::GetFrameHeight() + 4.0f;

  // 3. Position it at the very bottom
  // Pos.y + Size.y gives us the bottom edge
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - footerHeight));

  // 4. Stretch it to the full width
  ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, footerHeight));

  // 5. Use Viewport ID to ensure it handles multi-monitor/Retina correctly
  ImGui::SetNextWindowViewport(viewport->ID);

  // 6. Flags to lock it down (No title, no move, no resize)
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                  ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

  // Optional: Make it slightly transparent or a specific "Mac" gray
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 3.0f));

  if (ImGui::Begin("##MainStatusBar", nullptr, window_flags)) {
    ImGui::Text("Project: MySimulation");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), ICON_FA_CIRCLE " Engine Running");

    // Right-aligned section
    float indicatorWidth = 120.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - indicatorWidth);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  }
  ImGui::End();

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor();
}
void Editor::drawDockspace() {
  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Open Project")) {
      m_context.project.load();
      openScene(m_context.project.getConfig().start_scene);
    }
    if (ImGui::MenuItem("Save Project")) {
      m_context.project.save();
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  m_scene_hierarchy_panel.onImGuiRender(m_selection_context);
  m_asset_browser_panel.onImGuiRender(m_selection_context);
  m_properties_panel.onImGuiRender(m_selection_context, m_active_scene.get());
  m_console_panel.onImGuiRender();
  DrawMainStatusBar();
}

static bool ToggleButton(const char* label, bool toggled, ImVec2 size = ImVec2(0, 0)) {
  if (toggled) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
  bool pressed = ImGui::Button(label, size);
  if (toggled) ImGui::PopStyleColor();

  return pressed;
}

void Editor::drawViewport() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);

  if (ImGui::BeginMenuBar()) {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    ImVec2 size(28, 0);
    if (ToggleButton(ICON_FA_STOP, m_context.clock.isStopped(), size)) m_context.clock.stop();
    if (ToggleButton(ICON_FA_PLAY, m_context.clock.isPlaying(), size)) m_context.clock.play();
    if (ToggleButton(ICON_FA_PAUSE, m_context.clock.isPaused(), size)) m_context.clock.pause();
    if (ImGui::Button(ICON_FA_FORWARD_FAST, size)) m_context.clock.step();
    ImGui::PopStyleVar();

    ImGui::TextDisabled("|");
    ImGui::Text("speed");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("##speed", &m_context.clock.speed, 0.0f, 3.0f, "%.1fx");
    ImGui::TextDisabled("|");
    ImGui::Text("t = %.3fs", m_context.clock.simTime());
    ImGui::EndMenuBar();
  }

  m_viewport_focused = ImGui::IsWindowFocused();
  m_viewport_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

  ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
  if (m_viewport_size.x != viewport_panel_size.x || m_viewport_size.y != viewport_panel_size.y) {
    m_viewport_size = {viewport_panel_size.x, viewport_panel_size.y};
    m_editor_camera.setViewportSize(m_viewport_size.x, m_viewport_size.y);
  }

  uint32_t texture_id = m_framebuffer->getColorAttachmentRendererID();
  // NOLINTNEXTLINE
  ImGui::Image(reinterpret_cast<void*>(texture_id), viewport_panel_size, ImVec2(0, 1), ImVec2(1, 0));

  drawAxisGizmo();

  // Overlay toggles
  // ImVec2 viewport_min = ImGui::GetItemRectMin();
  // ImGui::SetCursorScreenPos(ImVec2(viewport_min.x + 8.0f, viewport_min.y + 8.0f));
  // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
  // if (m_grid_overlay) {
  //   bool show = m_grid_overlay->enabled;
  //   if (ImGui::Checkbox("Grid", &show)) m_grid_overlay->enabled = show;
  // }
  // ImGui::PopStyleVar();

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
      m_selection_context = m_active_scene->getEntityFromHandle(static_cast<entt::entity>(entity_id));
    }
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void Editor::drawAxisGizmo() {
  // if (!m_axis_gizmo_enabled) return;

  // Position the gizmo in the bottom-left of the viewport panel.
  ImVec2 panel_max = ImGui::GetItemRectMax();
  constexpr float kSize = 50.0f;
  constexpr float kOffset = 16.0f;
  ImVec2 origin = {panel_max.x - kOffset - kSize, panel_max.y - ImGui::GetItemRectSize().y + kOffset + kSize};

  // Rotation-only view — strip translation from the camera view matrix.
  glm::mat4 rot_view = glm::mat4(glm::mat3(m_editor_camera.getViewMatrix()));
  glm::mat4 proj = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, -10.f, 10.f);
  glm::mat4 vp = proj * rot_view;

  // Project a direction vector into the gizmo's 2D panel space.
  auto project = [&](glm::vec3 dir) -> ImVec2 {
    glm::vec4 clip = vp * glm::vec4(dir, 0.0f);
    // Y is flipped: +Y in GL clip space == up, but ImGui Y grows downward.
    return {origin.x + clip.x * kSize, origin.y - clip.y * kSize};
  };

  ImDrawList* dl = ImGui::GetWindowDrawList();
  constexpr float kThickness = 2.0f;

  struct {
    glm::vec3 dir;
    ImU32 color;
    const char* label;
  } axes[] = {
      {{1, 0, 0}, IM_COL32(210, 60, 60, 255), "X"},
      {{0, 1, 0}, IM_COL32(60, 210, 60, 255), "Y"},
      {{0, 0, 1}, IM_COL32(60, 60, 210, 255), "Z"},
  };

  for (auto& [dir, color, label] : axes) {
    ImVec2 tip = project(dir);
    dl->AddLine(origin, tip, color, kThickness);
    dl->AddCircleFilled(tip, 4.0f, color);
    dl->AddText({tip.x + 5.0f, tip.y - 7.0f}, color, label);
  }
}

void Editor::handleShortcuts() {
  auto& io = ImGui::GetIO();
  if (io.WantTextInput) return;

  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
    m_context.project.save();
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
    m_context.project.load();
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
    m_context.project.New();
  }
  if (ImGui::IsKeyPressed(ImGuiKey_F)) {
    // Focus on selected object
    // if (std::holds_alternative<Entity>(m_selection_context)) {
    //   auto entity = std::get<Entity>(m_selection_context);
    //   if (entity) {
    //     auto tc = entity.getComponent<TransformComponent>();
    //     m_editor_camera.focusEntity(tc.Translation, glm::compMax(tc.Scale) * 5);
    //   }
    // }
  }
}

void Editor::drawDebugPanel() {
  ImGui::Begin("Framebuffer Debug");

  ImVec2 size(256, 144);
  uint32_t debug_tex = m_framebuffer->getDebugEntityIDTextureID();
  ImGui::Text("Entity ID buffer");
  // NOLINTNEXTLINE
  ImGui::Image(reinterpret_cast<ImTextureID>(debug_tex), size, ImVec2(0, 1), ImVec2(1, 0));

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

static void setTheme() {
  ImVec4* colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.48f, 0.27f, 0.54f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.98f, 0.50f, 0.40f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.98f, 0.50f, 0.67f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.48f, 0.27f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.98f, 0.50f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.88f, 0.47f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.98f, 0.50f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.26f, 0.98f, 0.50f, 0.40f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.98f, 0.50f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.98f, 0.33f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.26f, 0.98f, 0.50f, 0.31f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.98f, 0.50f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.98f, 0.50f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.75f, 0.32f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.75f, 0.32f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.98f, 0.50f, 0.20f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.98f, 0.50f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.98f, 0.50f, 0.95f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.98f, 0.50f, 0.80f);
  colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.58f, 0.33f, 0.86f);
  colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.68f, 0.37f, 1.00f);
  colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.98f, 0.50f, 1.00f);
  colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.15f, 0.10f, 0.97f);
  colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.42f, 0.24f, 1.00f);
  colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.98f, 0.50f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
  colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.98f, 0.50f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.98f, 0.50f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.98f, 0.52f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
