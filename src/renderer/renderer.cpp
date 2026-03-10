#include "renderer/renderer.h"

#include <glad/gl.h>

#include "utils/filesystem.h"

Renderer::SceneData Renderer::s_scene_data;
std::shared_ptr<Shader> Renderer::s_outline_shader;

void Renderer::init() {
  auto vertex_path = FileSystem::resolvePath("engine://shaders/outline.vert");
  auto frag_path = FileSystem::resolvePath("engine://shaders/outline.frag");
  s_outline_shader = std::make_shared<Shader>(vertex_path.c_str(), frag_path.c_str());
}

void Renderer::beginScene(const glm::mat4& view_proj, const glm::vec2& viewport_size) {
  s_scene_data.view_proj = view_proj;
  s_scene_data.viewport_size = viewport_size;

  glEnable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  glClear(GL_STENCIL_BUFFER_BIT);
}

void Renderer::endScene() {
  glDisable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  glEnable(GL_DEPTH_TEST);
}

void Renderer::submit(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material,
                      const glm::mat4& transform, int entity_id, bool mark_stencil) {
  if (!mesh || !material) return;

  if (mark_stencil) {
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  } else {
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  }

  material->setProperty("u_proj_view", s_scene_data.view_proj);
  material->setProperty("u_model", transform);
  material->setProperty("u_entity_id", entity_id);
  material->bind();
  mesh->render();
}

void Renderer::submitOutline(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, const glm::vec3& color,
                             float width_pixels) {
  if (!mesh || !s_outline_shader) return;

  glColorMaski(1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glStencilMask(0x00);
  glDisable(GL_DEPTH_TEST);

  s_outline_shader->use();
  s_outline_shader->setMat4("u_proj_view", s_scene_data.view_proj);
  s_outline_shader->setMat4("u_model", transform);
  s_outline_shader->setVec2("u_viewport_size", s_scene_data.viewport_size);
  s_outline_shader->setFloat("u_outline_width", width_pixels);
  s_outline_shader->setVec3("u_outline_color", color);

  mesh->render();

  glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDisable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  glEnable(GL_DEPTH_TEST);
}