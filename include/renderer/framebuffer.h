#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "utils/utils.h"

enum class FramebufferTextureFormat {
  None = 0,

  RGBA8,
  RED_INTEGER,

  Color = RGBA8,

  DEPTH24STENCIL8,

  Depth = DEPTH24STENCIL8,
};

struct FramebufferTextureSpecification {
  FramebufferTextureSpecification() = default;
  FramebufferTextureSpecification(FramebufferTextureFormat format) : texture_format(format) {}

  FramebufferTextureFormat texture_format = FramebufferTextureFormat::None;
};

struct FramebufferAttachmentSpecification {
  FramebufferAttachmentSpecification() = default;
  FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
      : attachments(attachments) {}

  std::vector<FramebufferTextureSpecification> attachments;
};

struct FramebufferSpecification {
  uint32_t width = 0, height = 0;
  FramebufferAttachmentSpecification attachments;
  uint32_t samples = 1;
};

class Framebuffer {
 public:
  Framebuffer(const FramebufferSpecification& spec);
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

  FramebufferSpecification m_spec;
  std::vector<FramebufferTextureSpecification> m_color_attachment_specs;
  FramebufferTextureSpecification m_depth_attachment_spec = FramebufferTextureFormat::None;

  std::vector<unsigned int> m_color_attachments;
  unsigned int m_depth_attachment = 0;
};