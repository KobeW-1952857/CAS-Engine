#include "renderer/systems/mesh_renderer_system.h"

#include "core/asset_manager.h"
#include "renderer/material.h"
#include "renderer/primitives/mesh.h"
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
    if (!mesh || !material) continue;

    auto transform = view.get<TransformComponent>(entity).getWorldTransform(ctx.scene, e);

    bool selected = e == ctx.selected_entity;
    if (selected)
      ctx.renderer.setStencilWrite(1);
    else
      ctx.renderer.clearStencilWrite();

    Renderer::PerDrawUBO draw_data{.model = transform};
    ctx.renderer.uploadPerDrawUBO(draw_data);

    material->bind();
    mesh->draw();
  }
}

void MeshRenderSystem::onEntityIDPass(entt::registry& registry, const RenderContext& ctx) {
  auto view = registry.view<TransformComponent, MeshComponent>();
  for (auto entity : view) {
    auto mesh = ctx.assets.getAsset<Mesh>(view.get<MeshComponent>(entity).mesh_handle);
    auto transform = view.get<TransformComponent>(entity).getWorldTransform(ctx.scene, Entity(entity, &registry));
    if (!mesh) continue;

    Renderer::PerDrawUBO draw_data{.model = transform};
    ctx.renderer.uploadPerDrawUBO(draw_data);

    ctx.renderer.submitEntityID(*mesh, static_cast<int>(entity));
  }
}

void MeshRenderSystem::onOutlinePass(entt::registry& registry, const RenderContext& ctx) {
  if (!ctx.selected_entity || !ctx.selected_entity.hasComponent<MeshComponent>()) return;
  auto mesh = ctx.assets.getAsset<Mesh>(ctx.selected_entity.getComponent<MeshComponent>().mesh_handle);
  if (!mesh) return;
  auto transform =
      ctx.selected_entity.getComponent<TransformComponent>().getWorldTransform(ctx.scene, ctx.selected_entity);
  Renderer::PerDrawUBO draw_data{.model = transform};
  ctx.renderer.uploadPerDrawUBO(draw_data);
  ctx.renderer.submitOutline(*mesh);
}