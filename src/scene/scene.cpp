#include "scene/scene.h"

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <fstream>
#include <glm/gtx/string_cast.hpp>

#include "Nexus/Log.h"
#include "core/asset_manager.h"
#include "core/uuid.h"
#include "editor/editor_camera.h"
#include "renderer/material.h"
#include "renderer/render_context.h"
#include "renderer/renderer.h"
#include "scene/component_type_list.h"
#include "scene/components.h"
#include "scene/entity.h"

Scene::Scene() { type = AssetType::Scene; }
Scene::Scene(AssetManager* assets, Renderer* renderer) : m_assets(assets), m_renderer(renderer) {
  type = AssetType::Scene;
}

void Scene::init(AssetManager* assets, Renderer* renderer) {
  m_assets = assets;
  m_renderer = renderer;
}

Entity Scene::createEntity(const std::string& name) { return createEntity(UUID(), name); }

Entity Scene::createEntity(UUID id, const std::string& name) {
  auto entity = Entity(m_registry.create(), &m_registry);

  entity.addComponent<TagComponent>(name.empty() ? "Empty entity" : name);
  entity.addComponent<IDComponent>(id);
  entity.addComponent<TransformComponent>();

  m_entity_map.emplace(id, entity);

  modified = true;

  return entity;
}

void Scene::destroyEntity(Entity entity) {
  if (!entity) return;

  UUID id = entity.getComponent<IDComponent>().ID;
  m_registry.destroy(entity);
  m_entity_map.erase(id);

  modified = true;
}

std::vector<Entity> Scene::getEntities() const {
  std::vector<Entity> entities;
  for (auto entity_id : m_registry.view<entt::entity>()) {
    Entity entity(entity_id, const_cast<entt::registry*>(&m_registry));
    entities.push_back(entity);
  }
  return entities;
}

Entity Scene::getEntity(UUID id) const {
  if (m_entity_map.find(id) == m_entity_map.end()) return Entity();
  return m_entity_map.at(id);
}

Entity Scene::getEntityFromHandle(entt::entity handle) const {
  return Entity(handle, const_cast<entt::registry*>(&m_registry));
}

void Scene::onRender(Entity selected_entity, const EditorCamera& camera, const glm::vec2& viewport_size) {
  Renderer::SceneData scene_data{.viewport_size = viewport_size,
                                 .cam_pos = camera.getPosition(),
                                 .view_proj = camera.getViewProjectionMatrix(),
                                 .projection = camera.getProjectionMatrix(),
                                 .view = camera.getViewMatrix()};
  m_renderer->beginScene(scene_data);

  RenderContext ctx{*m_renderer, *m_assets, selected_entity, *this};

  m_renderer->beginColorPass();
  for (auto& system : m_render_systems) system->onColorPass(m_registry, ctx);

  for (auto& system : m_render_systems) system->onOutlinePass(m_registry, ctx);

  m_renderer->beginEntityIDPass();
  for (auto& system : m_render_systems) system->onEntityIDPass(m_registry, ctx);

  m_renderer->endScene();
}

void Scene::onImGuiRender() {}

std::shared_ptr<Scene> AssetTraits<Scene>::load(const std::filesystem::path& path, AssetManager& assets) {
  auto scene = std::make_shared<Scene>(&assets, assets.getRenderer());

  YAML::Node data;
  try {
    data = YAML::LoadFile(path);
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load scene file '{}'. Error: {}", path.string(), e.what());
    return nullptr;
  }

  Nexus::Logger::debug("Deserializing scene '{}'", path.stem().string());

  auto entities = data["Entities"];
  if (entities) {
    for (const auto& entity : entities) {
      auto uuid = entity["ID"].as<UUID>();

      std::string name = entity["TagComponent"] ? entity["TagComponent"]["Tag"].as<std::string>() : "";

      Nexus::Logger::trace("Deserializing entity with ID = {}, Name = {}", static_cast<uint64_t>(uuid), name);
      Entity deserialized_entity = scene->createEntity(uuid, name);

      forEachComponentType([&]<typename T>() { T::deserialize(entity, deserialized_entity); });
    }
  }

  return scene;
}

static void serializeEntity(YAML::Emitter& out, Entity entity) {
  out << YAML::BeginMap;

  Nexus::Logger::debug("Serializing entity {}", entity.getComponent<TagComponent>().tag);

  forEachComponentType([&]<typename T>() {
    if (auto* c = entity.tryGetComponent<T>()) T::serialize(out, *c);
  });

  out << YAML::EndMap;
}

void AssetTraits<Scene>::save(const Scene& scene, const std::filesystem::path& path, AssetManager& assets) {
  Nexus::Logger::debug("Serializing scene to '{}", path.string());

  YAML::Emitter out;
  out << YAML::BeginMap;

  out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

  for (const auto& entity : scene.getEntities()) serializeEntity(out, entity);

  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(path);
  fout << out.c_str();
}

void AssetTraits<Scene>::initializeNew(Scene& scene, AssetManager& assets) {
  scene.init(&assets, assets.getRenderer());
}