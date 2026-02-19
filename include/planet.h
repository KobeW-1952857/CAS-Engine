#pragma once

#include "texture.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "mesh.h"
#include "utils.h"

class Planet {
 public:
  Planet(std::string name, Color color, Planet* primary = nullptr, float distance = 0.0f, float orbit = 0.0f,
         Point position = Point(0, 0, 0), glm::quat rotation = glm::quat(1, 0, 0, 0), float scale = 1.0f);

  void addTexture(const std::string& path);

  void update(float dt, float simulation_speed);

  void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
  void drawUI();

 private:
  Point calculatePosition();

 private:
  std::string m_name;

  // Transform
  Point m_position;      // in solar system
  glm::quat m_rotation;  // around own axis
  float m_scale;
  float m_intensity = 1.0f;

  Mesh m_mesh;

  // Material
  Texture m_texture;
  bool m_hasTexture;
  Color m_color;

  // Orbit
  Planet* m_primary;
  float m_orbit;
  float m_distance;
};