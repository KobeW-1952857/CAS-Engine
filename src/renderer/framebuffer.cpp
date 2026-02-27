#include "renderer/framebuffer.h"

#include <Nexus/Log.h>
#include <glad/gl.h>

#include "utils/utils.h"

Framebuffer::Framebuffer(int width, int height) : m_size(width, height) {
  glGenFramebuffers(1, &m_fbo);
  bind();

  glEnable(GL_DEPTH_TEST);

  genTexture(width, height);
  genRenderbuffer(width, height);

  attach();
  clear();

  unbind();
}

Framebuffer::~Framebuffer() {
  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_texture);
  glDeleteRenderbuffers(1, &m_rbo);
}

void Framebuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_size.x, m_size.y);
}

void Framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::clear(Color color) {
  glClearColor(color.x, color.y, color.z, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::resize(int width, int height) {
  if (m_size.x == width && m_size.y == height) return;

  if (width == 0 || height == 0) {
    Nexus::Logger::warn("Framebuffer size is 0, setting to 1x1");
    width = 1;
    height = 1;
  }

  m_size = {width, height};
  glViewport(0, 0, width, height);

  glDeleteTextures(1, &m_texture);
  genTexture(width, height);

  glDeleteRenderbuffers(1, &m_rbo);
  genRenderbuffer(width, height);

  attach();
}

void Framebuffer::genTexture(int width, int height) {
  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::genRenderbuffer(int width, int height) {
  glGenRenderbuffers(1, &m_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::attach() {
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Nexus::Logger::error("Framebuffer is not complete: {}", glCheckFramebufferStatus(GL_FRAMEBUFFER));
  }
}