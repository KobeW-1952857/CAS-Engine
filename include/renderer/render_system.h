#pragma once
#include <entt.hpp>

#include "renderer/render_context.h"

class Renderer;

class IRenderSystem {
 public:
  virtual ~IRenderSystem() = default;

  virtual void onColorPass(entt::registry&, const RenderContext&) = 0;
  virtual void onEntityIDPass(entt::registry&, const RenderContext&) = 0;
  virtual void onOutlinePass(entt::registry&, const RenderContext&) {};
};