#include "scene/scene.h"

#include <glm/gtx/string_cast.hpp>

#include "core/asset_manager.h"
#include "core/uuid.h"
#include "renderer/material.h"
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

void Scene::onRender(const glm::mat4& view_proj) {
  {
    auto view = m_registry.view<TransformComponent, MaterialComponent, MeshComponent>();
    for (auto entity : view) {
      auto [transform, material_component, mesh_component] =
          view.get<TransformComponent, MaterialComponent, MeshComponent>(entity);

      auto mesh = AssetManager::getAsset<Mesh>(mesh_component.mesh_handle);
      auto material = AssetManager::getAsset<Material>(material_component.material_handle);

      if (mesh && material) {
        material->setProperty("u_proj_view", view_proj);
        material->setProperty("u_model", transform.getTransform());

        material->bind();
        mesh->render();
      }
    }
  }
}

void Scene::onImGuiRender() {}