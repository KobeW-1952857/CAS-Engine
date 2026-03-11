#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"

class Renderer {
 public:
  struct SceneData {
    glm::vec2 viewport_size;
    glm::mat4 view_proj;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cam_pos;
  };
  static void init();

  static void beginScene(SceneData scene_data);
  static void endScene();

  static void submit(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material,
                     const glm::mat4& transform, int entity_id, bool mark_stencil = false);

  static void submitOutline(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform,
                            const glm::vec3& color = {1.0f, 0.5f, 0.0f}, float outline_width = 10.0f);

 private:
  static SceneData s_scene_data;
  static std::shared_ptr<Shader> s_outline_shader;
};