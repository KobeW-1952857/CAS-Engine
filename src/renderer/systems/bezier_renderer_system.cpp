#include "renderer/systems/bezier_renderer_system.h"

#include "scene/components.h"
#include "utils/filesystem.h"

BezierRendererSystem::BezierRendererSystem(FileSystem& fs) {
  m_bezier_shader = std::make_shared<Shader>(fs.resolvePath("engine://shaders/bezier"), fs);
  // TODO(kobe): use a different shader for the control point handles
  m_handle_shader = std::make_shared<Shader>(fs.resolvePath("engine://shaders/engine/handle"), fs);
}

void BezierRendererSystem::onColorPass(entt::registry& registry, const RenderContext& ctx) {
  auto view = registry.view<BezierComponent>();
  if (view.begin() == view.end()) return;

  m_bezier_shader->use();

  for (auto entity : view) {
    auto& bc = view.get<BezierComponent>(entity);

    Renderer::PerDrawUBO draw_data{.model = glm::mat4(1.0f)};
    ctx.renderer.uploadPerDrawUBO(draw_data);
    m_bezier_shader->setVec3("u_color", bc.color);

    bc.patch->update({bc.control_points[0], bc.control_points[1], bc.control_points[2], bc.control_points[3]});
    bc.patch->draw();
  }
}

void BezierRendererSystem::onEntityIDPass(entt::registry& registry, const RenderContext& ctx) {}

void BezierRendererSystem::onOutlinePass(entt::registry& registry, const RenderContext& ctx) {
  if (!ctx.selected_entity || !ctx.selected_entity.hasComponent<BezierComponent>()) return;
  auto view = registry.view<BezierComponent>();
  if (view.begin() == view.end()) return;

  m_handle_shader->use();

  for (auto entity : view) {
    auto& bc = view.get<BezierComponent>(entity);

    Renderer::PerDrawUBO draw_data{.model = glm::mat4(1.0f)};
    ctx.renderer.uploadPerDrawUBO(draw_data);

    m_handle_shader->setVec3("u_color", bc.color);
    m_handle_shader->setFloat("u_radius", 0.1f);

    // glPointSize(5.0f);
    bc.points->update({bc.control_points[0], bc.control_points[1], bc.control_points[2], bc.control_points[3]});
    bc.points->draw();
    // glPointSize(1.0f);
  }
}