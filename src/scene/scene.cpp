#include "scene/scene.h"

#include <cstdint>
#include <glm/gtx/string_cast.hpp>

#include "core/asset_manager.h"
#include "core/uuid.h"
#include "renderer/material.h"
#include "renderer/renderer.h"
#include "scene/components.h"
#include "scene/entity.h"

Entity Scene::createEntity(const std::string& name) { return createEntity(UUID(), name); }

Entity Scene::createEntity(UUID id, const std::string& name) {
  auto entity = Entity(m_registry.create(), weak_from_this());

  entity.addComponent<TagComponent>(name.empty() ? "Empty entity" : name);
  entity.addComponent<IDComponent>(id);
  entity.addComponent<TransformComponent>();

  m_entity_map.emplace(id, entity);

  return entity;
}

void Scene::destroyEntity(Entity entity) {
  if (!entity) return;

  UUID id = entity.getComponent<IDComponent>().ID;
  m_registry.destroy(entity);
  m_entity_map.erase(id);
}

void Scene::onRender(const glm::mat4& view_proj, const glm::vec2& viewport_size, Entity selected_entity) {
  Renderer::beginScene(view_proj, viewport_size);

  auto view = m_registry.view<TransformComponent, MaterialComponent, MeshComponent>();
  for (auto entity : view) {
    Entity e(entity, shared_from_this());
    auto mesh = AssetManager::getAsset<Mesh>(e.getComponent<MeshComponent>().mesh_handle);
    auto material = AssetManager::getAsset<Material>(e.getComponent<MaterialComponent>().material_handle);
    bool is_selected = (e == selected_entity);

    Renderer::submit(mesh, material, e.getComponent<TransformComponent>().getTransform(), static_cast<int>(entity),
                     is_selected);
  }

  if (selected_entity) {
    auto& tc = selected_entity.getComponent<TransformComponent>();
    auto mesh = AssetManager::getAsset<Mesh>(selected_entity.getComponent<MeshComponent>().mesh_handle);
    Renderer::submitOutline(mesh, tc.getTransform());
  }

  Renderer::endScene();
}

void Scene::renderEntity(Entity entity, const glm::mat4& view_proj) {
  if (!entity || !entity.hasComponent<MeshComponent>() || !entity.hasComponent<MaterialComponent>()) return;
  auto& tc = entity.getComponent<TransformComponent>();
  auto& mc = entity.getComponent<MeshComponent>();
  auto& matc = entity.getComponent<MaterialComponent>();

  auto mesh = AssetManager::getAsset<Mesh>(mc.mesh_handle);
  auto material = AssetManager::getAsset<Material>(matc.material_handle);

  if (mesh && material) {
    material->setProperty("u_proj_view", view_proj);
    material->setProperty("u_model", tc.getTransform());
    material->setProperty("u_entity_id", static_cast<int>(static_cast<uint32_t>(entity)));

    material->bind();
    mesh->render();
  }
}

void Scene::renderEntityOutline(Entity entity, const glm::mat4& view_proj) {
  if (!entity || !entity.hasComponent<MeshComponent>()) return;
}

void Scene::onImGuiRender() {}