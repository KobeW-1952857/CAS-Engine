#include "renderer/renderer.h"

#include <glad/gl.h>

#include "utils/filesystem.h"
#include "utils/utils.h"

void Renderer::init(FileSystem& filesystem) {
  s_outline_shader = std::make_shared<Shader>(filesystem.resolvePath("engine://shaders/outline"), filesystem);
  s_entity_id_shader =
      std::make_shared<Shader>(filesystem.resolvePath("engine://shaders/engine/entity_id"), filesystem);

  glGenBuffers(1, &s_per_frame_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, s_per_frame_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUBO), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, s_per_frame_ubo);

  glGenBuffers(1, &s_per_draw_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, s_per_draw_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerDrawUBO), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, s_per_draw_ubo);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::beginScene(SceneData scene_data) {
  s_scene_data = scene_data;

  glEnable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  glClear(GL_STENCIL_BUFFER_BIT);

  glm::vec3 light_offset = glm::vec3(1.5f, 2.0f, 0.0f);
  glm::mat4 inv_view = glm::inverse(scene_data.view);
  glm::vec3 light_pos = glm::vec3(inv_view * glm::vec4(light_offset, 1.0f));

  PerFrameUBO frame_data{
      .proj_view = scene_data.view_proj,
      .cam_pos = scene_data.cam_pos,
      .light_pos = light_pos,
      .viewport_size = scene_data.viewport_size,
  };

  glBindBuffer(GL_UNIFORM_BUFFER, s_per_frame_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerFrameUBO), &frame_data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  checkOpenGLError("Renderer::beginScene");
}

void Renderer::beginColorPass() {
  GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE};
  glDrawBuffers(2, buffers);
  checkOpenGLError("Renderer::beginColorPass");
}
void Renderer::beginEntityIDPass() {
  glDepthFunc(GL_LEQUAL);
  GLenum buffers[] = {GL_NONE, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, buffers);
  checkOpenGLError("Renderer::beginEntityIDPass");
}

void Renderer::endScene() {
  glDepthFunc(GL_LESS);
  GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, buffers);
  glDisable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  glEnable(GL_DEPTH_TEST);
  checkOpenGLError("Renderer::endScene");
}

void Renderer::submit(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material,
                      const glm::mat4& transform, bool mark_stencil) {
  if (!mesh || !material) return;

  if (mark_stencil) {
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  } else {
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  }

  PerDrawUBO draw_data{.model = transform};
  glBindBuffer(GL_UNIFORM_BUFFER, s_per_draw_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerDrawUBO), &draw_data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  material->bind();
  mesh->render();

  checkOpenGLError("renderer::submit");
}

void Renderer::submitEntityID(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, int entity_id) {
  if (!mesh) return;
  PerDrawUBO draw_data{.model = transform};

  glBindBuffer(GL_UNIFORM_BUFFER, s_per_draw_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerDrawUBO), &draw_data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  s_entity_id_shader->use();
  s_entity_id_shader->setInt("u_entity_id", entity_id);
  mesh->render();
  checkOpenGLError("renderer::submitEntityID");
}

void Renderer::submitOutline(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, const glm::vec3& color,
                             float outline_width) {
  if (!mesh || !s_outline_shader) return;

  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glStencilMask(0x00);
  glDisable(GL_DEPTH_TEST);

  s_outline_shader->use();
  s_outline_shader->setFloat("u_outline_width", outline_width);
  s_outline_shader->setVec3("u_outline_color", color);
  PerDrawUBO draw_data{
      .model = transform,
  };
  glBindBuffer(GL_UNIFORM_BUFFER, s_per_draw_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerDrawUBO), &draw_data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  mesh->render();

  glDisable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  glEnable(GL_DEPTH_TEST);
}