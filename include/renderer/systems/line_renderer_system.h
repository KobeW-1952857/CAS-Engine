#pragma once

#include "renderer/render_system.h"

class FileSystem;
class Shader;

class LineRendererSystem : public IRenderSystem {
 public:
  explicit LineRendererSystem(FileSystem& fs);
  void onColorPass(entt::registry& registry, const RenderContext& render_context) override;
  void onEntityIDPass(entt::registry& registry, const RenderContext& render_context) override;
  void onOutlinePass(entt::registry& registry, const RenderContext& render_context) override;

 private:
  std::shared_ptr<Shader> m_line_shader;
};