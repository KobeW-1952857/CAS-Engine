#include "renderer/systems/mesh_renderer_system.h"

#include "core/asset_manager.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/render_context.h"
#include "renderer/renderer.h"
#include "scene/components.h"
#include "scene/entity.h"

void MeshRenderSystem::onColorPass(entt::registry& registry, const RenderContext& ctx) {
  auto view = registry.view<TransformComponent, MaterialComponent, MeshComponent>();

  for (auto entity : view) {
    Entity e(entity, &registry);

    auto mesh = ctx.assets.getAsset<Mesh>(view.get<MeshComponent>(entity).mesh_handle);
    auto material = ctx.assets.getAsset<Material>(view.get<MaterialComponent>(entity).material_handle);
    auto transform = view.get<TransformComponent>(entity).getTransform();

    ctx.renderer.submit(mesh, material, transform, entity == ctx.selected_entity);
  }
}

void MeshRenderSystem::onEntityIDPass(entt::registry& registry, const RenderContext& ctx) {
  auto view = registry.view<TransformComponent, MeshComponent>();
  for (auto entity : view) {
    auto mesh = ctx.assets.getAsset<Mesh>(view.get<MeshComponent>(entity).mesh_handle);
    auto transform = view.get<TransformComponent>(entity).getTransform();

    ctx.renderer.submitEntityID(mesh, transform, static_cast<int>(entity));
  }
}

void MeshRenderSystem::onOutlinePass(entt::registry& registry, const RenderContext& ctx) {
  if (!ctx.selected_entity || !ctx.selected_entity.hasComponent<MeshComponent>()) return;
  auto mesh = ctx.assets.getAsset<Mesh>(ctx.selected_entity.getComponent<MeshComponent>().mesh_handle);
  auto transform = ctx.selected_entity.getComponent<TransformComponent>().getTransform();

  ctx.renderer.submitOutline(mesh, transform);
}