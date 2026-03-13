#pragma once

#include <entt.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "core/asset.h"
#include "core/asset_traits.h"
#include "core/uuid.h"
#include "renderer/render_system.h"
#include "scene/entity.h"

class AssetManager;
class EditorCamera;
class Renderer;
class SceneHierarchy;

class Scene : public Asset {
 public:
  Scene();
  Scene(AssetManager* assets, Renderer* renderer);
  void init(AssetManager* assets, Renderer* renderer);

  void registerRenderSystem(std::unique_ptr<IRenderSystem> system) { m_render_systems.push_back(std::move(system)); }

  Entity createEntity(const std::string& name = "");
  Entity createEntity(UUID id, const std::string& name = "");
  void destroyEntity(Entity entity);
  std::vector<Entity> getEntities() const;
  Entity getEntity(UUID id) const;
  Entity getEntityFromHandle(entt::entity handle) const;

  void onUpdate(float dt);
  void onRender(Entity selected_entity, const EditorCamera& camera, const glm::vec2& viewport_size);
  void renderEntity(Entity entity, const glm::mat4& view_proj);
  void renderEntityOutline(Entity entity, const glm::mat4& view_proj);
  void onImGuiRender();

 private:
  entt::registry m_registry;
  std::unordered_map<UUID, Entity> m_entity_map;
  AssetManager* m_assets = nullptr;
  Renderer* m_renderer = nullptr;

  std::vector<std::unique_ptr<IRenderSystem>> m_render_systems;

  friend SceneHierarchy;
};

template <>
struct AssetTraits<Scene> {
  static constexpr AssetType type = AssetType::Scene;
  static constexpr const char* default_name = "New Scene";
  static constexpr const char* extension = ".casscene";

  static bool matchesExtension(std::string_view ext) { return ext == extension; }

  static std::shared_ptr<Scene> load(const std::filesystem::path& path, AssetManager& assets);
  static void save(const Scene& scene, const std::filesystem::path& path, AssetManager& assets);
  static void initializeNew(Scene& asset, AssetManager& assets);
};