#include "renderer/systems/latice_renderer_system.h"

#include "scene/components/latice_component.h"
#include "scene/components/transform_component.h"

LaticeRendererSystem::LaticeRendererSystem(FileSystem& fs) {
  m_line_shader = std::make_shared<Shader>(fs.resolvePath("engine://shaders/line"), fs);
  m_line_primitive = std::make_shared<LinePrimitive>();
  m_point_primitive = std::make_shared<PointPrimitive>();
}

void LaticeRendererSystem::onColorPass(entt::registry& registry, const RenderContext& ctx) {
  auto view = registry.view<LaticeComponent, TransformComponent>();
  if (view.begin() == view.end()) return;
  m_line_shader->use();

  for (auto entity : view) {
    Entity e(entity, &registry);
    auto& lc = view.get<LaticeComponent>(entity);
    auto& tc = view.get<TransformComponent>(entity);

    Renderer::PerDrawUBO draw_data{.model = tc.getWorldTransform(ctx.scene, e)};
    ctx.renderer.uploadPerDrawUBO(draw_data);
    m_line_shader->setVec3("u_color", glm::vec3(1.0f, 1.0f, 1.0f));

    glPointSize(5.0f);
    m_point_primitive->update(lc.control_points);
    m_point_primitive->draw();
    glPointSize(1.0f);

    /* TODO (Kobe)
     * We are currently drawing the latice by generating line vertices on the CPU and uploading them to the GPU every
     * frame, which is not very efficient. We should ideally only update the line vertices when the latice control
     * points change, but for simplicity we are doing it every frame for now.
     */

    // Draw lines between control points in the same row
    std::vector<glm::vec3> line_vertices;
    for (int i = 0; i < lc.rows; ++i) {
      for (int j = 0; j < lc.cols - 1; ++j) {
        for (int k = 0; k < lc.layers; ++k) {
          int index1 = i * lc.cols * lc.layers + j * lc.layers + k;
          int index2 = i * lc.cols * lc.layers + (j + 1) * lc.layers + k;
          line_vertices.push_back(lc.control_points[index1]);
          line_vertices.push_back(lc.control_points[index2]);
        }
      }
    }
    m_line_primitive->update(line_vertices);
    m_line_primitive->draw();
    // Draw lines between control points in the same column
    line_vertices.clear();
    for (int i = 0; i < lc.rows - 1; ++i) {
      for (int j = 0; j < lc.cols; ++j) {
        for (int k = 0; k < lc.layers; ++k) {
          int index1 = i * lc.cols * lc.layers + j * lc.layers + k;
          int index2 = (i + 1) * lc.cols * lc.layers + j * lc.layers + k;
          line_vertices.push_back(lc.control_points[index1]);
          line_vertices.push_back(lc.control_points[index2]);
        }
      }
    }
    m_line_primitive->update(line_vertices);
    m_line_primitive->draw();
    // Draw lines between control points in the same layer
    line_vertices.clear();
    for (int i = 0; i < lc.rows; ++i) {
      for (int j = 0; j < lc.cols; ++j) {
        for (int k = 0; k < lc.layers - 1; ++k) {
          int index1 = i * lc.cols * lc.layers + j * lc.layers + k;
          int index2 = i * lc.cols * lc.layers + j * lc.layers + (k + 1);
          line_vertices.push_back(lc.control_points[index1]);
          line_vertices.push_back(lc.control_points[index2]);
        }
      }
    }
    m_line_primitive->update(line_vertices);
    m_line_primitive->draw();
  }
}

void LaticeRendererSystem::onEntityIDPass(entt::registry& registry, const RenderContext& ctx) {}

void LaticeRendererSystem::onOutlinePass(entt::registry& registry, const RenderContext& ctx) {}