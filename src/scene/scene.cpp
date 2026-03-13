#include "scene/scene.h"

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <tuple>

#include "core/asset_manager.h"
#include "core/asset_traits.h"
#include "core/uuid.h"
#include "editor/editor_camera.h"
#include "renderer/material.h"
#include "renderer/renderer.h"
#include "scene/component_traits.h"
#include "scene/component_type_list.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "utils/yaml_extension.h"

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
  auto entity = Entity(m_registry.create(), this);

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

std::vector<Entity> Scene::getEntities() {
  std::vector<Entity> entities;
  for (auto entity_id : m_registry.view<entt::entity>()) {
    Entity entity(entity_id, this);
    entities.push_back(entity);
  }
  return entities;
}

Entity Scene::getEntity(UUID id) const {
  if (m_entity_map.find(id) == m_entity_map.end()) return Entity();
  return m_entity_map.at(id);
}

void Scene::onRender(Entity selected_entity, const EditorCamera& camera, const glm::vec2& viewport_size) {
  Renderer::SceneData scene_data{.viewport_size = viewport_size,
                                 .cam_pos = camera.getPosition(),
                                 .view_proj = camera.getViewProjectionMatrix(),
                                 .projection = camera.getProjectionMatrix(),
                                 .view = camera.getViewMatrix()};
  m_renderer->beginScene(scene_data);
  auto view = m_registry.view<TransformComponent, MaterialComponent, MeshComponent>();

  m_renderer->beginColorPass();
  for (auto entity : view) {
    Entity e(entity, this);
    auto mesh = m_assets->getAsset<Mesh>(e.getComponent<MeshComponent>().mesh_handle);
    auto material = m_assets->getAsset<Material>(e.getComponent<MaterialComponent>().material_handle);
    auto transform = e.getComponent<TransformComponent>().getTransform();

    m_renderer->submit(mesh, material, transform, e == selected_entity);
  }

  if (selected_entity && selected_entity.hasComponent<MeshComponent>()) {
    auto& tc = selected_entity.getComponent<TransformComponent>();
    auto mesh = m_assets->getAsset<Mesh>(selected_entity.getComponent<MeshComponent>().mesh_handle);
    m_renderer->submitOutline(mesh, tc.getTransform());
  }

  m_renderer->beginEntityIDPass();
  for (auto entity : view) {
    Entity e(entity, this);
    auto mesh = m_assets->getAsset<Mesh>(e.getComponent<MeshComponent>().mesh_handle);
    auto transform = e.getComponent<TransformComponent>().getTransform();

    m_renderer->submitEntityID(mesh, transform, static_cast<int>(static_cast<uint32_t>(e)));
  }

  m_renderer->endScene();
}

void Scene::renderEntity(Entity entity, const glm::mat4& view_proj) {
  if (!entity || !entity.hasComponent<MeshComponent>() || !entity.hasComponent<MaterialComponent>()) return;
  auto& tc = entity.getComponent<TransformComponent>();
  auto& mc = entity.getComponent<MeshComponent>();
  auto& matc = entity.getComponent<MaterialComponent>();

  auto mesh = m_assets->getAsset<Mesh>(mc.mesh_handle);
  auto material = m_assets->getAsset<Material>(matc.material_handle);

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

std::shared_ptr<Scene> AssetTraits<Scene>::load(const std::filesystem::path& path, AssetManager& assets) {
  auto scene = std::make_shared<Scene>(&assets, assets.getRenderer());

  YAML::Node data;
  try {
    data = YAML::LoadFile(path);
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load scene file '{}'. Error: {}", path.string(), e.what());
    return nullptr;
  }

  if (!data["Scene"]) {
    Nexus::Logger::error("Invalid scene file '{}': Missing 'Scene' node.", path.string());
    return nullptr;
  }

  Nexus::Logger::debug("Deserializing scene '{}'", path.stem().string());

  auto entities = data["Entities"];
  if (entities) {
    for (const auto& entity : entities) {
      auto uuid = entity["ID"].as<UUID>();

      std::string name = entity["TagComponent"] ? entity["TagComponent"]["Tag"].as<std::string>() : "";

      Nexus::Logger::trace("Deserializing entity with ID = {}, Name = {}", static_cast<uint64_t>(uuid), name);
      Entity deserializedEntity = scene->createEntity(uuid, name);

      std::apply([&](auto... tags) { (ComponentTraits<decltype(tags)>::deserialize(entity, deserializedEntity), ...); },
                 AllComponentsTypes{});
    }
  }

  return scene;
}

static void serializeEntity(YAML::Emitter& out, Entity entity) {
  out << YAML::BeginMap;
  std::apply([&](auto... tags) { (ComponentTraits<decltype(tags)>::serialize(out, entity), ...); },
             AllComponentsTypes{});
  out << YAML::EndMap;
}

void AssetTraits<Scene>::save(Scene& scene, const std::filesystem::path& path, AssetManager assets) {
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