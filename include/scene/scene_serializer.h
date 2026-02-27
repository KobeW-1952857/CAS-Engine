#pragma once

#include "scene/scene.h"
#include "yaml-cpp/yaml.h"

class SceneSerializer {
 public:
  SceneSerializer() = default;
  SceneSerializer(const std::shared_ptr<Scene>& scene) : m_scene(scene) {}
  ~SceneSerializer() = default;

  void setContext(const std::shared_ptr<Scene>& scene) { m_scene = scene; }

  void serialize(const std::string& filepath);

  bool deserialize(const std::string& filepath);

 private:
  void serializeEntity(YAML::Emitter& out, Entity entity);

  std::shared_ptr<Scene> m_scene;
};