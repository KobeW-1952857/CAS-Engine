#pragma once

#include <glm/glm.hpp>

#include "utils.h"

class Framebuffer {
 public:
  Framebuffer(int width, int height);
  ~Framebuffer();

  void bind();
  void unbind();
  void clear(Color color = {0.98, 0.98, 0.98, 1.0});

  void resize(int width, int height);

  unsigned int getTexture() const { return m_texture; }

 private:
  void genTexture(int width, int height);
  void genRenderbuffer(int width, int height);
  void attach();

  unsigned int m_fbo;
  unsigned int m_texture;
  unsigned int m_rbo;
  glm::ivec2 m_size;
};