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

class AssetManager;
class EditorCamera;
class Entity;
class Renderer;
class SceneHierarchy;

class Scene : public Asset, public std::enable_shared_from_this<Scene> {
 public:
  Scene();
  Scene(AssetManager* assets, Renderer* renderer);
  void init(AssetManager* assets, Renderer* renderer);

  Entity createEntity(const std::string& name = "");
  Entity createEntity(UUID id, const std::string& name = "");
  void destroyEntity(Entity entity);
  std::vector<Entity> getEntities();
  Entity getEntity(UUID id) const;

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

  friend Entity;
  friend SceneHierarchy;
  friend class SceneSerializer;
};

template <>
struct AssetTraits<Scene> {
  static constexpr AssetType type = AssetType::Scene;
  static constexpr const char* default_name = "New Scene";
  static constexpr const char* extension = ".casscene";

  static bool matchesExtension(std::string_view ext) { return ext == extension; }

  static std::shared_ptr<Scene> load(const std::filesystem::path& path, AssetManager& assets);
  static void save(Scene& scene, const std::filesystem::path& path, AssetManager assets);
  static void initializeNew(Scene& asset, AssetManager& assets);
};