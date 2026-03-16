#include "renderer/systems/line_renderer_system.h"

#include <glm/glm.hpp>

#include "scene/components.h"

LineRendererSystem::LineRendererSystem(FileSystem& fs) {
  m_line_shader = std::make_shared<Shader>(fs.resolvePath("engine://shaders/line"), fs);
}

void LineRendererSystem::onColorPass(entt::registry& registry, const RenderContext& ctx) {
  auto view = registry.view<LineComponent>();
  if (view.begin() == view.end()) return;

  m_line_shader->use();

  for (auto entity : view) {
    auto& lc = view.get<LineComponent>(entity);

    Renderer::PerDrawUBO draw_data{.model = glm::mat4(1.0f)};
    ctx.renderer.uploadPerDrawUBO(draw_data);
    m_line_shader->setVec3("u_color", lc.color);

    // glLineWidth(lc.thickness);
    lc.line->update(lc.p0, lc.p1);
    lc.line->draw();
    // glLineWidth(1.0f);
  }
}

void LineRendererSystem::onEntityIDPass(entt::registry& registry, const RenderContext& ctx) {}

void LineRendererSystem::onOutlinePass(entt::registry& registry, const RenderContext& ctx) {
  if (!ctx.selected_entity || !ctx.selected_entity.hasComponent<LineComponent>()) return;
  auto view = registry.view<LineComponent>();
  if (view.begin() == view.end()) return;

  m_line_shader->use();

  for (auto entity : view) {
    auto& lc = view.get<LineComponent>(entity);

    Renderer::PerDrawUBO draw_data{.model = glm::mat4(1.0f)};
    ctx.renderer.uploadPerDrawUBO(draw_data);
    m_line_shader->setVec3("u_color", lc.color);

    glPointSize(lc.thickness);
    lc.points->update({lc.p0, lc.p1});
    lc.points->draw();
    glPointSize(1.0f);
  }
}