#include "editor/grid_overlay.h"

#include <glad/gl.h>

#include "utils/filesystem.h"

GridOverlay::GridOverlay(FileSystem& fs) {
  m_shader = std::make_shared<Shader>(fs.resolvePath("engine://shaders/editor/grid"), fs);
  glGenVertexArrays(1, &m_vao);
}

GridOverlay::~GridOverlay() {
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void GridOverlay::render() {
  if (!enabled) return;

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);
  glStencilMask(0x00);

  m_shader->use();
  m_shader->setFloat("u_scale", scale);

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glStencilMask(0xFF);
  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}