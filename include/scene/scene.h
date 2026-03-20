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
#include "scene/logic_system.h"

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
  void registerLogicSystem(std::unique_ptr<ILogicSystem> system) { m_logic_systems.push_back(std::move(system)); }

  Entity createEntity(const std::string& name = "");
  Entity createEntity(UUID id, const std::string& name = "");
  void destroyEntity(Entity entity);
  std::vector<Entity> getEntities() const;
  Entity getEntity(UUID id) const;
  Entity getEntityFromHandle(entt::entity handle) const;

  template <typename... Components>
  void forEachEntity(auto&& func) {
    auto view = m_registry.view<Components...>();
    for (auto entity : view) {
      Entity e(entity, &m_registry);
      func(e);
    }
  }
  template <typename... Components>
  void forEachEntityWithAny(auto&& func) {
    // auto view = m_registry.view<entt::entity, Components...>(entt::exclude_t<>());
    // for (auto entity : view) {
    //   Entity e(entity, &m_registry);
    //   func(e);
    // }
    std::unordered_set<entt::entity> visited;

    (
        [&] {
          auto view = m_registry.view<Components>();
          for (auto entity : view) {
            if (visited.contains(entity)) return;
            visited.insert(entity);
            func(Entity(entity, &m_registry));
          }
        }(),
        ...);
  }

  void setParent(Entity child, Entity parent);
  void unParent(Entity child);

  void onUpdate(float dt);
  void onRender(Entity selected_entity, const EditorCamera& camera, const glm::vec2& viewport_size,
                std::function<void()> overlay_pass = {});
  void onImGuiRender();

 private:
  entt::registry m_registry;
  std::unordered_map<UUID, Entity> m_entity_map;
  AssetManager* m_assets = nullptr;
  Renderer* m_renderer = nullptr;

  std::vector<std::unique_ptr<IRenderSystem>> m_render_systems;
  std::vector<std::unique_ptr<ILogicSystem>> m_logic_systems;

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