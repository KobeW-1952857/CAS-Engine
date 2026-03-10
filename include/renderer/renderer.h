#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"

class Renderer {
 public:
  static void init();

  static void beginScene(const glm::mat4& view_proj, const glm::vec2& viewport_size);
  static void endScene();

  static void submit(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material,
                     const glm::mat4& transform, int entity_id, bool mark_stencil = false);

  static void submitOutline(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform,
                            const glm::vec3& color = {1.0f, 0.5f, 0.0f}, float outline_width = 10.0f);

 private:
  struct SceneData {
    glm::mat4 view_proj;
    glm::vec2 viewport_size;
  };

  static SceneData s_scene_data;
  static std::shared_ptr<Shader> s_outline_shader;
};