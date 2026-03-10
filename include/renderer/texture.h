#pragma once

#include <cstdint>
#include <glm/glm.hpp>

// NOLINTNEXTLINE
enum class FramebufferTextureFormat {
  None = 0,

  RGBA8,
  RED_INTEGER,

  DEPTH24STENCIL8,

  Color = RGBA8,
  Depth = DEPTH24STENCIL8,
};

class Texture {
 public:
  Texture();
  ~Texture();

  void bind(uint32_t slot = 0) const;
  void unbind() const;

  void setData(void* data, uint32_t size);
  void resize(uint32_t width, uint32_t height);

  uint32_t getWidth() const { return m_width; }
  uint32_t getHeight() const { return m_height; }
  uint32_t getRendererID() const { return m_rendererID; }
  FramebufferTextureFormat getFormat() const { return m_format; }

  // Static factory methods for different texture types/formats
  static std::shared_ptr<Texture> create2D(FramebufferTextureFormat format, uint32_t width, uint32_t height);
  static std::shared_ptr<Texture> createDepthStencil(FramebufferTextureFormat format, uint32_t width, uint32_t height);

 private:
  uint32_t m_rendererID = 0;
  uint32_t m_width = 0, m_height = 0;
  FramebufferTextureFormat m_format = FramebufferTextureFormat::None;

  void createGLTexture(FramebufferTextureFormat format, uint32_t width, uint32_t height);
  void deleteGLTexture();
};