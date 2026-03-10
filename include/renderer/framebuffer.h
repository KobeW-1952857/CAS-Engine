#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "renderer/texture.h"
#include "utils/utils.h"

struct FramebufferTextureSpecification {
  FramebufferTextureSpecification() = default;
  FramebufferTextureSpecification(FramebufferTextureFormat format) : texture_format(format) {}  // NOLINT

  FramebufferTextureFormat texture_format = FramebufferTextureFormat::None;
  // TODO(Kobe): Add filtering options
};

struct FramebufferSpecification {
  uint32_t width = 0, height = 0;
  std::vector<FramebufferTextureSpecification> attachments;
  uint32_t samples = 1;
  bool swapChainTarget = false;
};

class Framebuffer {
 public:
  explicit Framebuffer(const FramebufferSpecification& spec);
  ~Framebuffer();

  void bind();
  void unbind();
  void clear(Color color = {0.98, 0.98, 0.98, 1.0});
  void clearAttachment(uint32_t attachmentIndex, int value);

  void resize(uint32_t width, uint32_t height);
  int readPixel(uint32_t attachmentIndex, int x, int y);

  uint32_t getColorAttachmentRendererID(uint32_t index = 0) const;
  uint32_t getDepthAttachmentRendererID() const;

  const FramebufferSpecification& getSpecification() const { return m_spec; }

  uint32_t getDebugEntityIDTextureID();
  const std::unordered_map<int, std::tuple<uint8_t, uint8_t, uint8_t>>& getDebugEntityColorMap() const {
    return m_debug_color_map;
  }

 private:
  void invalidate();

  uint32_t m_rendererID = 0;

  uint32_t m_debug_entity_id_texture = 0;
  std::unordered_map<int, std::tuple<uint8_t, uint8_t, uint8_t>> m_debug_color_map;

  FramebufferSpecification m_spec;
  std::vector<std::shared_ptr<Texture>> m_colorAttachments;
  std::shared_ptr<Texture> m_depthAttachment;
};