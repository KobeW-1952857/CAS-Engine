#include "editor/editor.h"

#include <ImGui/imgui.h>
#include <ImGuizmo.h>
#include <Nexus/Log.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <variant>

#include "core/asset_manager.h"
#include "core/project.h"
#include "editor/panels/debug_icons.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer/systems/bezier_renderer_system.h"
#include "renderer/systems/latice_renderer_system.h"
#include "renderer/systems/line_renderer_system.h"
#include "renderer/systems/mesh_renderer_system.h"
#include "scene/components/transform_component.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scene/systems/line_follower_system.h"
#include "utils/IconBindings.h"
#include "utils/utils.h"

void EditorPrefs::load(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) return;

  try {
    YAML::Node data = YAML::LoadFile(path.string());
    auto recents = data["RecentProjects"];
    if (recents) {
      for (const auto& proj : recents) {
        recent_projects.push_back(proj.as<std::string>());
      }
    }
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load editor prefs: {}", e.what());
  }
}

void EditorPrefs::save(const std::filesystem::path& path) const {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "RecentProjects" << YAML::Value << YAML::BeginSeq;
  for (const auto& proj : recent_projects) {
    out << proj;
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(path);
  fout << out.c_str();
}

void EditorPrefs::addRecentProject(const std::string& project_path) {
  std::string safe_path = project_path;
  // If the project is already in the list, remove it so we can push it to the top
  auto it = std::find(recent_projects.begin(), recent_projects.end(), project_path);
  if (it != recent_projects.end()) {
    recent_projects.erase(it);
  }

  // Insert at the front (most recent)
  recent_projects.insert(recent_projects.begin(), std::move(safe_path));

  // Optional: Keep only the 10 most recent projects
  if (recent_projects.size() > 10) {
    recent_projects.pop_back();
  }
}

void loadIconFonts(const std::filesystem::path& fonts_dir, float font_size) {
  ImGuiIO& io = ImGui::GetIO();
  Nexus::Logger::trace("Loading icon fonts from directory: {}", fonts_dir.string());

  ImFontConfig config;
  config.MergeMode = true;
  config.PixelSnapH = true;

  for (const auto& entry : std::filesystem::directory_iterator(fonts_dir)) {
    Nexus::Logger::trace("Checking font file: {}", entry.path().filename().string());
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != ".ttf") continue;
    Nexus::Logger::trace("Loading icon font: {}", entry.path().filename().string());

    io.Fonts->AddFontFromFileTTF(entry.path().string().c_str(), font_size, &config);
  }

  io.Fonts->Build();
}

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
  scene->registerRenderSystem(std::make_unique<LaticeRendererSystem>(m_context.filesystem));

  scene->registerLogicSystem(std::make_unique<LineFollowerSystem>());
}

static void setTheme();

void Editor::init() {
  setTheme();
  ImGuiIO& io = ImGui::GetIO();
  ImFontConfig config;
  config.PixelSnapH = true;
  ImWchar ranges[] = {0x0000, 0xF1AF0};

  io.Fonts->AddFontFromFileTTF(m_context.filesystem.resolvePath("engine://fonts/ComfortaaNerdFont-Regular.ttf").c_str(),
                               16.0f, &config, ranges);
  // loadIconFonts(m_context.filesystem.resolvePath("engine://fonts/icons"), 16.0f);

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
  m_prefs.load(m_prefs_path);
}

void Editor::setContext(const std::shared_ptr<Scene>& scene) {
  m_active_scene = scene;
  m_scene_hierarchy_panel.setContext(scene);
  m_context.active_scene = scene.get();
}

void Editor::onUpdate(float dt) {
  m_context.clock.advanceFrame(dt);

  if (m_viewport_focused || m_viewport_hovered || m_editor_camera.isFocussing())
    m_editor_camera.onUpdate(dt, ImGui::GetIO().WantCaptureKeyboard);

  if (m_active_scene) {
    for (uint64_t i = 0; i < m_context.clock.pendingTicks(); ++i) m_active_scene->onUpdate(m_context.clock.tickDt());
  }
  m_asset_browser_panel.onUpdate(dt);

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

void Editor::drawLauncher() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 center = viewport->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(700, 450));

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

  ImGui::Begin("CAS Engine Launcher", nullptr, window_flags);

  // --- LEFT COLUMN: Engine Info & Actions ---
  ImGui::BeginChild("LeftPanel", ImVec2(200, 0), true);

  // Engine Title / Logo Area
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);  // Assuming you might have a larger font loaded
  ImGui::TextColored(ImVec4(0.0f, 0.70f, 0.45f, 1.0f), ICON_FA_CUBES " CAS Engine");
  ImGui::PopFont();
  ImGui::Separator();
  ImGui::Spacing();

  // Action Buttons
  if (ImGui::Button(ICON_FA_FOLDER_OPEN " Open Project", ImVec2(-1, 40))) {
    m_context.project.load();
    m_prefs.addRecentProject(m_context.project.getConfig().path);
    m_prefs.save(m_prefs_path);
    openScene(m_context.project.getConfig().start_scene);
  }

  ImGui::Spacing();

  if (ImGui::Button(ICON_FA_PLUS " New Project", ImVec2(-1, 40))) {
    m_context.project.New();
  }
  ImGui::EndChild();

  ImGui::SameLine();

  // --- RIGHT COLUMN: Recent Projects ---
  ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);
  ImGui::TextDisabled("Recent Projects");
  ImGui::Separator();
  ImGui::Spacing();

  if (m_prefs.recent_projects.empty()) {
    ImGui::TextDisabled("No recent projects found.");
  } else {
    for (const auto& proj_path : m_prefs.recent_projects) {
      // Display just the folder name or the full path
      std::filesystem::path p(proj_path);
      std::string display_name = std::format(ICON_FA_FILE_CODE "  {}", p.filename().string().c_str());

      if (ImGui::Selectable(display_name.c_str(), false, 0, ImVec2(0, 30))) {
        m_prefs.addRecentProject(proj_path);
        m_prefs.save(m_prefs_path);

        m_context.project.load(p);
        openScene(m_context.project.getConfig().start_scene);
        break;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", proj_path.c_str());
      }
    }
  }

  ImGui::EndChild();

  ImGui::End();
  ImGui::PopStyleVar(2);

  drawNerdFontRanges(nullptr);
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
    drawDebugIconWindow(nullptr);
  } else {
    drawLauncher();
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

  ImGuiStyle& style = ImGui::GetStyle();

  ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_MenuBarBg]);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 3.0f));

  if (ImGui::Begin("##MainStatusBar", nullptr, window_flags)) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Project: MySimulation");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), ICON_FA_CIRCLE " Engine Running");
    ImGui::SameLine();

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
  auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
  auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
  auto viewportOffset = ImGui::GetWindowPos();
  ImVec2 viewport_bounds_min = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
  ImVec2 viewport_bounds_max = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};

  if (ImGui::BeginMenuBar()) {
    ImVec2 size(28, 0);
    if (ToggleButton(ICON_FA_STOP, m_context.clock.isStopped(), size)) m_context.clock.stop();
    if (ToggleButton(ICON_FA_PLAY, m_context.clock.isPlaying(), size)) m_context.clock.play();
    if (ToggleButton(ICON_FA_PAUSE, m_context.clock.isPaused(), size)) m_context.clock.pause();
    if (ImGui::Button(ICON_FA_FORWARD_FAST, size)) m_context.clock.step();

    ImGui::TextDisabled("|");
    if (ToggleButton("T", m_gizmo_type == ImGuizmo::TRANSLATE, size)) m_gizmo_type = ImGuizmo::TRANSLATE;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Translate (W)");
    if (ToggleButton("R", m_gizmo_type == ImGuizmo::ROTATE, size)) m_gizmo_type = ImGuizmo::ROTATE;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate (E)");
    if (ToggleButton("S", m_gizmo_type == ImGuizmo::SCALE, size)) m_gizmo_type = ImGuizmo::SCALE;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scale (R)");

    ImGui::TextDisabled("|");
    ImGui::Text("speed");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("##speed", &m_context.clock.speed, 0.0f, 3.0f, "%.1fx");
    // ImGui::TextDisabled("|");
    // ImGui::Text("gizmo scale");
    // ImGui::SameLine();
    // ImGui::SetNextItemWidth(80.0f);
    // ImGui::SliderFloat("##gizmosize", &m_gizmo_size, 0.01f, 0.5f, "%.2f");
    ImGui::TextDisabled("|");
    ImGui::Text("t = %.3fs", m_context.clock.simTime());
    ImGui::EndMenuBar();
  }

  m_viewport_focused = ImGui::IsWindowFocused();

  ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
  if (m_viewport_size.x != viewport_panel_size.x || m_viewport_size.y != viewport_panel_size.y) {
    m_viewport_size = {viewport_panel_size.x, viewport_panel_size.y};
    m_editor_camera.setViewportSize(m_viewport_size.x, m_viewport_size.y);
  }

  uint32_t texture_id = m_framebuffer->getColorAttachmentRendererID();
  // NOLINTNEXTLINE
  ImGui::Image(reinterpret_cast<void*>(texture_id), viewport_panel_size, ImVec2(0, 1), ImVec2(1, 0));

  m_viewport_hovered = ImGui::IsItemHovered();

  drawAxisGizmo();

  // !TEMP
  ImGuizmo::BeginFrame();
  ImGuizmo::SetGizmoSizeClipSpace(m_gizmo_size);
  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(viewport_bounds_min.x, viewport_bounds_min.y, viewport_bounds_max.x - viewport_bounds_min.x,
                    viewport_bounds_max.y - viewport_bounds_min.y);
  if (std::holds_alternative<Entity>(m_selection_context)) {
    auto entity = std::get<Entity>(m_selection_context);
    m_gizmo_system.onImGuiRender(entity, *m_active_scene, m_editor_camera, m_gizmo_type);
  }
  // !END TEMP

  if (ImGui::IsMouseClicked(0) && m_viewport_hovered && !ImGuizmo::IsUsingAny()) {
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
      {{1, 0, 0}, IM_COL32(255, 89, 89, 255), "X"},
      {{0, 1, 0}, IM_COL32(115, 217, 89, 255), "Y"},
      {{0, 0, 1}, IM_COL32(64, 115, 217, 255), "Z"},
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

  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) m_context.project.save();
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) m_context.project.load();
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) m_context.project.New();

  if (ImGui::IsKeyPressed(ImGuiKey_F)) {
    if (std::holds_alternative<Entity>(m_selection_context)) {
      auto entity = std::get<Entity>(m_selection_context);
      if (entity && entity.hasComponent<TransformComponent>()) {
        auto& tc = entity.getComponent<TransformComponent>();
        glm::mat4 world_transform = tc.getWorldTransform(*m_active_scene, entity);
        glm::vec3 world_pos = glm::vec3(world_transform[3]);  // Column 3 represents the translation
        float max_scale = std::max({std::abs(tc.scale.x), std::abs(tc.scale.y), std::abs(tc.scale.z)});
        m_editor_camera.focusEntity(world_pos, max_scale * 5.0f);
      }
    }
  }

  // Prevent changing modes while right-clicking (flying the camera)
  if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
    if (ImGui::IsKeyPressed(ImGuiKey_W))
      m_gizmo_type = ImGuizmo::TRANSLATE;
    else if (ImGui::IsKeyPressed(ImGuiKey_E))
      m_gizmo_type = ImGuizmo::ROTATE;
    else if (ImGui::IsKeyPressed(ImGuiKey_R))
      m_gizmo_type = ImGuizmo::SCALE;
  }
}

void Editor::drawDebugPanel() {
  ImGui::Begin("Framebuffer Debug");

  ImVec2 size(256, 144);
  uint32_t debug_tex = m_framebuffer->getDebugEntityIDTextureID();
  ImGui::Text("Entity ID buffer");
  // NOLINTNEXTLINE
  ImGui::Image(static_cast<ImTextureID>(debug_tex), size, ImVec2(0, 1), ImVec2(1, 0));

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

ImVec4 OKLCHtoImVec4(float L, float C, float h, float alpha = 1.0f) {
  float h_rad = h * (3.14159265f / 180.0f);

  float a = C * std::cos(h_rad);
  float b = C * std::sin(h_rad);

  float l_ = L + 0.3963377774f * a + 0.2158037573f * b;
  float m_ = L - 0.1055613458f * a - 0.0638541728f * b;
  float s_ = L - 0.0894841775f * a - 1.2914855480f * b;

  float l = l_ * l_ * l_;
  float m = m_ * m_ * m_;
  float s = s_ * s_ * s_;

  float r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
  float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
  float b_ = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

  return ImVec4(std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b_, 0.0f, 1.0f), alpha);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Mono Teal — dark (H 160) — H 160 — OKLCH perceptual palette
//  Generated from oklch() stops mapped to ImGui semantic roles.
//  Call once during init, after ImGui::CreateContext().
// ──────────────────────────────────────────────────────────────────────────────
void StyleMonoTealDark(ImGuiStyle* dst) {
  ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
  ImVec4* colors = style->Colors;

  // text
  colors[ImGuiCol_Text] = ImVec4(0.82f, 0.94f, 0.87f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.08f, 0.38f, 0.25f, 1.00f);

  // backgrounds
  colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.01f, 0.01f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.01f, 0.03f, 0.02f, 0.96f);

  // borders
  colors[ImGuiCol_Border] = ImVec4(0.08f, 0.38f, 0.25f, 0.28f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // frame (InputText, Slider, Checkbox bg)
  colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.08f, 0.06f, 0.70f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.08f, 0.14f, 0.11f, 0.80f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.14f, 0.22f, 0.18f, 1.00f);

  // title bar
  colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.01f, 0.01f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.08f, 0.06f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.01f, 0.01f, 0.75f);

  // menu bar
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.04f, 0.08f, 0.06f, 1.00f);

  // scrollbar
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.01f, 0.01f, 0.85f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.14f, 0.22f, 0.18f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.08f, 0.38f, 0.25f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);

  // check mark & slider
  colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.70f, 0.45f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.70f, 0.45f, 1.00f);

  // buttons
  colors[ImGuiCol_Button] = ImVec4(0.00f, 0.55f, 0.34f, 0.35f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.55f, 0.34f, 0.65f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);

  // headers (CollapsingHeader, TreeNode, Selectable, MenuItem)
  colors[ImGuiCol_Header] = ImVec4(0.00f, 0.55f, 0.34f, 0.30f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.55f, 0.34f, 0.50f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.55f, 0.34f, 0.85f);

  // separators
  colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.22f, 0.18f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.55f, 0.34f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.70f, 0.45f, 1.00f);

  // resize grip
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.55f, 0.34f, 0.18f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.55f, 0.34f, 0.70f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);

  // tabs
  colors[ImGuiCol_Tab] = ImVec4(0.01f, 0.03f, 0.02f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.08f, 0.38f, 0.25f, 0.80f);
  colors[ImGuiCol_TabActive] = ImVec4(0.08f, 0.14f, 0.11f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.01f, 0.01f, 0.97f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.04f, 0.08f, 0.06f, 1.00f);

  // docking
  colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.55f, 0.34f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.01f, 0.03f, 0.02f, 1.00f);

  // plots
  colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.70f, 0.45f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.70f, 0.45f, 1.00f);

  // tables
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.04f, 0.08f, 0.06f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.14f, 0.22f, 0.18f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.08f, 0.14f, 0.11f, 0.50f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.82f, 0.94f, 0.87f, 0.04f);

  // misc
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.55f, 0.34f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.55f, 0.34f, 1.00f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.70f, 0.45f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.82f, 0.94f, 0.87f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.01f, 0.01f, 0.50f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.01f, 0.01f, 0.55f);

  // ── style vars ──────────────────────────────────────────────
  style->WindowPadding = ImVec2(10.0f, 8.0f);
  style->FramePadding = ImVec2(8.0f, 4.0f);
  style->CellPadding = ImVec2(6.0f, 4.0f);
  style->ItemSpacing = ImVec2(8.0f, 5.0f);
  style->ItemInnerSpacing = ImVec2(4.0f, 4.0f);
  style->ScrollbarSize = 12.0f;
  style->GrabMinSize = 10.0f;
  style->WindowBorderSize = 1.0f;
  style->ChildBorderSize = 1.0f;
  style->PopupBorderSize = 1.0f;
  style->FrameBorderSize = 0.0f;
  style->TabBorderSize = 0.0f;
  style->WindowRounding = 6.0f;
  style->ChildRounding = 4.0f;
  style->FrameRounding = 4.0f;
  style->PopupRounding = 4.0f;
  style->ScrollbarRounding = 4.0f;
  style->GrabRounding = 4.0f;
  style->TabRounding = 4.0f;
}

// ──────────────────────────────────────────────────────────────────────────────
//  Mono Teal — light (H 160) — H 160 — OKLCH perceptual palette
//  Generated from oklch() stops mapped to ImGui semantic roles.
//  Call once during init, after ImGui::CreateContext().
// ──────────────────────────────────────────────────────────────────────────────
void StyleMonoTealLight(ImGuiStyle* dst) {
  ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
  ImVec4* colors = style->Colors;

  // text
  colors[ImGuiCol_Text] = ImVec4(0.00f, 0.03f, 0.01f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.65f, 0.46f, 1.00f);

  // backgrounds
  colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.97f, 0.96f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.88f, 0.92f, 0.90f, 0.97f);

  // borders
  colors[ImGuiCol_Border] = ImVec4(0.54f, 0.75f, 0.63f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // frame (InputText, Slider, Checkbox bg)
  colors[ImGuiCol_FrameBg] = ImVec4(0.77f, 0.84f, 0.80f, 0.80f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.54f, 0.75f, 0.63f, 0.80f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.54f, 0.75f, 0.63f, 1.00f);

  // title bar
  colors[ImGuiCol_TitleBg] = ImVec4(0.77f, 0.84f, 0.80f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.77f, 0.84f, 0.80f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.88f, 0.92f, 0.90f, 0.75f);

  // menu bar
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.88f, 0.92f, 0.90f, 1.00f);

  // scrollbar
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.95f, 0.97f, 0.96f, 0.85f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.54f, 0.75f, 0.63f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.65f, 0.46f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);

  // check mark & slider
  colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.41f, 0.20f, 1.00f);

  // buttons
  colors[ImGuiCol_Button] = ImVec4(0.00f, 0.53f, 0.31f, 0.20f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.53f, 0.31f, 0.45f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.53f, 0.31f, 0.80f);

  // headers (CollapsingHeader, TreeNode, Selectable, MenuItem)
  colors[ImGuiCol_Header] = ImVec4(0.24f, 0.65f, 0.46f, 0.25f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.65f, 0.46f, 0.45f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.53f, 0.31f, 0.80f);

  // separators
  colors[ImGuiCol_Separator] = ImVec4(0.54f, 0.75f, 0.63f, 0.60f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.53f, 0.31f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.41f, 0.20f, 1.00f);

  // resize grip
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.53f, 0.31f, 0.18f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.53f, 0.31f, 0.70f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);

  // tabs
  colors[ImGuiCol_Tab] = ImVec4(0.77f, 0.84f, 0.80f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.65f, 0.46f, 0.55f);
  colors[ImGuiCol_TabActive] = ImVec4(0.54f, 0.75f, 0.63f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.88f, 0.92f, 0.90f, 0.97f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.77f, 0.84f, 0.80f, 1.00f);
  colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.54f, 0.75f, 0.63f, 1.00f);

  // docking
  colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.53f, 0.31f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.88f, 0.92f, 0.90f, 1.00f);

  // plots
  colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.41f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.41f, 0.20f, 1.00f);

  // tables
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.77f, 0.84f, 0.80f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.54f, 0.75f, 0.63f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.77f, 0.84f, 0.80f, 0.80f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.00f, 0.03f, 0.01f, 0.04f);

  // misc
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.53f, 0.31f, 0.28f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.53f, 0.31f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.95f, 0.97f, 0.96f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.03f, 0.01f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.03f, 0.01f, 0.40f);

  // ── style vars ──────────────────────────────────────────────
  style->WindowPadding = ImVec2(10.0f, 8.0f);
  style->FramePadding = ImVec2(8.0f, 4.0f);
  style->CellPadding = ImVec2(6.0f, 4.0f);
  style->ItemSpacing = ImVec2(8.0f, 5.0f);
  style->ItemInnerSpacing = ImVec2(4.0f, 4.0f);
  style->ScrollbarSize = 12.0f;
  style->GrabMinSize = 10.0f;
  style->WindowBorderSize = 1.0f;
  style->ChildBorderSize = 1.0f;
  style->PopupBorderSize = 1.0f;
  style->FrameBorderSize = 0.0f;
  style->TabBorderSize = 0.0f;
  style->WindowRounding = 6.0f;
  style->ChildRounding = 4.0f;
  style->FrameRounding = 4.0f;
  style->PopupRounding = 4.0f;
  style->ScrollbarRounding = 4.0f;
  style->GrabRounding = 4.0f;
  style->TabRounding = 4.0f;
}

static void setTheme() {
  StyleMonoTealLight(nullptr);

  // Setup nicer, softer ImGuizmo colors (Blender/Unity style)
  ImGuizmo::Style& gizmo_style = ImGuizmo::GetStyle();
  gizmo_style.Colors[ImGuizmo::DIRECTION_X] = ImVec4(0.85f, 0.25f, 0.35f, 1.00f);
  gizmo_style.Colors[ImGuizmo::DIRECTION_Y] = ImVec4(0.45f, 0.85f, 0.35f, 1.00f);
  gizmo_style.Colors[ImGuizmo::DIRECTION_Z] = ImVec4(0.25f, 0.45f, 0.85f, 1.00f);
  gizmo_style.Colors[ImGuizmo::PLANE_X] = ImVec4(0.85f, 0.25f, 0.35f, 0.38f);
  gizmo_style.Colors[ImGuizmo::PLANE_Y] = ImVec4(0.45f, 0.85f, 0.35f, 0.38f);
  gizmo_style.Colors[ImGuizmo::PLANE_Z] = ImVec4(0.25f, 0.45f, 0.85f, 0.38f);
  gizmo_style.Colors[ImGuizmo::SELECTION] = ImVec4(1.00f, 0.80f, 0.20f, 0.60f);
  gizmo_style.Colors[ImGuizmo::ROTATION_USING_BORDER] = ImVec4(1.00f, 0.80f, 0.20f, 1.00f);
  gizmo_style.Colors[ImGuizmo::ROTATION_USING_FILL] = ImVec4(1.00f, 0.80f, 0.20f, 0.50f);

  // Increase thickness for better visibility
  gizmo_style.TranslationLineThickness = 4.0f;
  gizmo_style.TranslationLineArrowSize = 8.0f;
  gizmo_style.RotationLineThickness = 3.0f;
  gizmo_style.RotationOuterLineThickness = 4.0f;
  gizmo_style.ScaleLineThickness = 4.0f;
  gizmo_style.ScaleLineCircleSize = 8.0f;
  // ImGuiStyle& style = ImGui::GetStyle();
  // ImVec4* colors = style.Colors;

  // // --- 1. Master Configuration ---
  // const float h = 160.0f;  // Emerald Hue
  // const float c = 0.16f;   // Vibrant Chroma
  // const float midC = 0.08f;
  // const float lowC = 0.015f;  // "Almost grey" for backgrounds (adds depth)

  // // --- 2. Neutral Scale (Lightness Steps) ---
  // const float L0 = 0.08f;  // Deepest (Popups/Frames)
  // const float L1 = 0.22f;  // Dark (Window Bg)
  // const float L2 = 0.30f;  // Mid-Dark (Child Bg / Tabs)
  // const float L3 = 0.50f;  // Light-Dark (Borders / Buttons)

  // // --- 3. Accent Scale (Brand Levels) ---
  // const float AL_Base = 0.75f;    // The "glow" emerald
  // const float AL_Hover = 0.85f;   // Brighter emerald
  // const float AL_Active = 0.65f;  // Pressed emerald

  // // --- 4. Semantic Definitions ---
  // const ImVec4 bgDeep = OKLCHtoImVec4(L0, lowC, h, 1.00f);
  // const ImVec4 bgMid = OKLCHtoImVec4(L1, lowC, h, 1.00f);
  // const ImVec4 bgLight = OKLCHtoImVec4(L2, lowC, h, 1.00f);
  // const ImVec4 borderCol = OKLCHtoImVec4(L3, lowC, h, 1.00f);

  // const ImVec4 accent = OKLCHtoImVec4(AL_Base, c, h, 1.00f);
  // const ImVec4 accentHi = OKLCHtoImVec4(AL_Hover, c + 0.02f, h, 1.00f);
  // const ImVec4 accentLo = OKLCHtoImVec4(AL_Active, c, h, 1.00f);
  // const ImVec4 accentTrans = OKLCHtoImVec4(AL_Base, c, h, 0.40f);

  // // --- 2. Functional Colors ---
  // colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.95f, 1.00f);
  // colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

  // // Backgrounds
  // colors[ImGuiCol_WindowBg] = bgMid;
  // colors[ImGuiCol_PopupBg] = bgDeep;
  // colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);  // Keep transparent
  // colors[ImGuiCol_Border] = borderCol;

  // // Header & Tabs
  // colors[ImGuiCol_TitleBg] = bgDeep;
  // colors[ImGuiCol_TitleBgActive] = bgMid;
  // colors[ImGuiCol_Header] = accentTrans;
  // colors[ImGuiCol_HeaderHovered] = OKLCHtoImVec4(AL_Base, c, h, 0.60f);
  // colors[ImGuiCol_HeaderActive] = accentLo;
  // colors[ImGuiCol_Tab] = bgLight;
  // colors[ImGuiCol_TabHovered] = accentLo;
  // colors[ImGuiCol_TabActive] = accent;
  // colors[ImGuiCol_TabUnfocusedActive] = OKLCHtoImVec4(L3, c * 0.5f, h, 1.00f);

  // // Interactive Elements
  // colors[ImGuiCol_FrameBg] = bgDeep;
  // colors[ImGuiCol_FrameBgHovered] = bgLight;
  // colors[ImGuiCol_FrameBgActive] = borderCol;
  // colors[ImGuiCol_Button] = accentTrans;
  // colors[ImGuiCol_ButtonHovered] = OKLCHtoImVec4(AL_Base, c, h, 0.60f);
  // colors[ImGuiCol_ButtonActive] = accentLo;
  // colors[ImGuiCol_CheckMark] = accent;
  // colors[ImGuiCol_SliderGrab] = accent;
  // colors[ImGuiCol_SliderGrabActive] = accentLo;

  // // Docking
  // colors[ImGuiCol_DockingPreview] = OKLCHtoImVec4(AL_Base, c, h, 0.70f);
  // // colors[ImGuiCol_DockingEmptyBg] = bgDeep;

  // // Separators
  // colors[ImGuiCol_Separator] = borderCol;
  // colors[ImGuiCol_SeparatorHovered] = accent;
  // colors[ImGuiCol_SeparatorActive] = accentLo;

  // // Resize Grips
  // colors[ImGuiCol_ResizeGrip] = OKLCHtoImVec4(AL_Base, c, h, 0.25f);
  // colors[ImGuiCol_ResizeGripHovered] = OKLCHtoImVec4(AL_Base, c, h, 0.65f);
  // colors[ImGuiCol_ResizeGripActive] = accentLo;

  // // Misc
  // colors[ImGuiCol_PlotLines] = accent;

  // // --- 3. Style Settings ---
  // style.WindowPadding = ImVec2(10.0f, 10.0f);
  // style.FramePadding = ImVec2(6.0f, 4.0f);
  // style.ItemSpacing = ImVec2(8.0f, 6.0f);
  // style.GrabMinSize = 12.0f;

  // style.WindowRounding = 6.0f;
  // style.ChildRounding = 4.0f;
  // style.FrameRounding = 4.0f;
  // style.PopupRounding = 4.0f;
  // style.TabRounding = 4.0f;
  // style.GrabRounding = 3.0f;

  // style.WindowBorderSize = 1.0f;
  // style.FrameBorderSize = 1.0f;
  // style.PopupBorderSize = 1.0f;
}
