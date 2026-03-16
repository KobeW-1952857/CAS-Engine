#pragma once

#include "renderer/render_system.h"
class FileSystem;
class Shader;

class BezierRendererSystem : public IRenderSystem {
 public:
  explicit BezierRendererSystem(FileSystem& fs);
  void onColorPass(entt::registry& registry, const RenderContext& render_context) override;
  void onEntityIDPass(entt::registry& registry, const RenderContext& render_context) override;
  void onOutlinePass(entt::registry& registry, const RenderContext& render_context) override;

 private:
  std::shared_ptr<Shader> m_bezier_shader;
  std::shared_ptr<Shader> m_handle_shader;
};