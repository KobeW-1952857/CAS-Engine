#pragma once

#include "renderer/primitives/line.h"
#include "renderer/primitives/point.h"
#include "renderer/render_system.h"
#include "renderer/shader.h"
#include "utils/filesystem.h"

class LaticeRendererSystem : public IRenderSystem {
 public:
  explicit LaticeRendererSystem(FileSystem& fs);
  void onColorPass(entt::registry& registry, const RenderContext& render_context) override;
  void onEntityIDPass(entt::registry& registry, const RenderContext& render_context) override;
  void onOutlinePass(entt::registry& registry, const RenderContext& render_context) override;

 private:
  std::shared_ptr<Shader> m_line_shader;
  std::shared_ptr<LinePrimitive> m_line_primitive;
  std::shared_ptr<PointPrimitive> m_point_primitive;
};