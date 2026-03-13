#pragma once

#include "renderer/render_system.h"

class MeshRenderSystem : public IRenderSystem {
 public:
  void onColorPass(entt::registry& registry, const RenderContext& render_context) override;
  void onEntityIDPass(entt::registry& registry, const RenderContext& render_context) override;
  void onOutlinePass(entt::registry& registry, const RenderContext& render_context) override;
};