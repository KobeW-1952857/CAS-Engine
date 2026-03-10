#include "renderer/texture.h"

#include <glad/gl.h>

#include <cassert>
#include <memory>

namespace {

GLenum FramebufferTextureFormatToGLInternalFormat(FramebufferTextureFormat format) {
  switch (format) {
    case FramebufferTextureFormat::RGBA8:
      return GL_RGBA8;
    case FramebufferTextureFormat::RED_INTEGER:
      return GL_R32I;
    case FramebufferTextureFormat::DEPTH24STENCIL8:
      return GL_DEPTH24_STENCIL8;
    case FramebufferTextureFormat::None:
      break;
  }
  assert(false && "Unknown FramebufferTextureFormat");
  return 0;
}

GLenum FramebufferTextureFormatToGLDataFormat(FramebufferTextureFormat format) {
  switch (format) {
    case FramebufferTextureFormat::RGBA8:
      return GL_RGBA;
    case FramebufferTextureFormat::RED_INTEGER:
      return GL_RED_INTEGER;
    case FramebufferTextureFormat::DEPTH24STENCIL8:
      return GL_DEPTH_STENCIL;
    case FramebufferTextureFormat::None:
      break;
  }
  assert(false && "Unknown FramebufferTextureFormat");
  return 0;
}

}  // namespace

Texture::Texture() = default;

Texture::~Texture() { deleteGLTexture(); }

void Texture::bind(uint32_t slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, m_rendererID);
}

void Texture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

void Texture::setData(void* data, uint32_t size) {
  uint32_t bpp = 0;
  GLenum dataFormat = 0;
  GLenum dataType = 0;

  switch (m_format) {
    case FramebufferTextureFormat::RGBA8:
      bpp = 4;
      dataFormat = GL_RGBA;
      dataType = GL_UNSIGNED_BYTE;
      break;
    case FramebufferTextureFormat::RED_INTEGER:
      bpp = 4;
      dataFormat = GL_RED_INTEGER;
      dataType = GL_INT;
      break;
    case FramebufferTextureFormat::DEPTH24STENCIL8:
      bpp = 4;
      dataFormat = GL_DEPTH_STENCIL;
      dataType = GL_UNSIGNED_INT_24_8;
      break;
    case FramebufferTextureFormat::None:
      assert(false && "Unsupported format for setData");
      return;
  }

  assert(size == m_width * m_height * bpp && "Texture data size mismatch!");

  glBindTexture(GL_TEXTURE_2D, m_rendererID);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, dataFormat, dataType, data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::resize(uint32_t width, uint32_t height) {
  if (m_format == FramebufferTextureFormat::None || (m_width == width && m_height == height)) {
    return;
  }

  deleteGLTexture();
  createGLTexture(m_format, width, height);
}

std::shared_ptr<Texture> Texture::create2D(FramebufferTextureFormat format, uint32_t width, uint32_t height) {
  auto texture = std::make_shared<Texture>();
  texture->createGLTexture(format, width, height);
  return texture;
}

std::shared_ptr<Texture> Texture::createDepthStencil(FramebufferTextureFormat format, uint32_t width, uint32_t height) {
  auto texture = std::make_shared<Texture>();
  texture->createGLTexture(format, width, height);
  return texture;
}

void Texture::createGLTexture(FramebufferTextureFormat format, uint32_t width, uint32_t height) {
  m_format = format;
  m_width = width;
  m_height = height;

  GLenum internalFormat = FramebufferTextureFormatToGLInternalFormat(format);
  GLenum dataFormat = FramebufferTextureFormatToGLDataFormat(format);
  GLenum type = GL_UNSIGNED_BYTE;

  if (format == FramebufferTextureFormat::DEPTH24STENCIL8 || format == FramebufferTextureFormat::Depth) {
    type = GL_UNSIGNED_INT_24_8;
  } else if (format == FramebufferTextureFormat::RED_INTEGER) {
    type = GL_INT;
  }

  glGenTextures(1, &m_rendererID);
  glBindTexture(GL_TEXTURE_2D, m_rendererID);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, dataFormat, type, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::deleteGLTexture() {
  if (m_rendererID) {
    glDeleteTextures(1, &m_rendererID);
    m_rendererID = 0;
  }
}