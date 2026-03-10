#pragma once

#include <entt.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "core/uuid.h"

class Entity;
class SceneHierarchy;

class Scene : public std::enable_shared_from_this<Scene> {
 public:
  Scene() = default;

  Entity createEntity(const std::string& name = "");
  Entity createEntity(UUID id, const std::string& name = "");
  void destroyEntity(Entity entity);

  void onUpdate(float dt);
  void onRender(const glm::mat4& view_proj, const glm::vec2& viewport_size, Entity selected_entity);
  void renderEntity(Entity entity, const glm::mat4& view_proj);
  void renderEntityOutline(Entity entity, const glm::mat4& view_proj);
  void onImGuiRender();

 private:
  entt::registry m_registry;
  std::unordered_map<UUID, Entity> m_entity_map;

  friend Entity;
  friend SceneHierarchy;
  friend class SceneSerializer;
};