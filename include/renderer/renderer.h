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
  static void beginColorPass();
  static void beginEntityIDPass();
  static void endScene();

  static void submit(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material,
                     const glm::mat4& transform, bool mark_stencil = false);
  static void submitEntityID(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, int entity_id);

  static void submitOutline(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform,
                            const glm::vec3& color = {1.0f, 0.5f, 0.0f}, float outline_width = 10.0f);

 private:
  struct PerFrameUBO {
    glm::mat4 proj_view;
    glm::vec3 cam_pos;
    float _pad0;
    glm::vec3 light_pos;
    float _pad1;
    glm::vec2 viewport_size;
  };
  struct PerDrawUBO {
    glm::mat4 model;
  };

  static SceneData s_scene_data;
  static std::shared_ptr<Shader> s_outline_shader;
  static std::shared_ptr<Shader> s_entity_id_shader;

  static GLuint s_per_frame_ubo;
  static GLuint s_per_draw_ubo;
};