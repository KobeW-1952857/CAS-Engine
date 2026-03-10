#include "renderer/framebuffer.h"

#include <Nexus/Log.h>
#include <glad/gl.h>

#include <cassert>

namespace {

bool IsDepthFormat(FramebufferTextureFormat format) {
  switch (format) {
    case FramebufferTextureFormat::DEPTH24STENCIL8:
      return true;
    default:
      return false;
  }
}

}  // namespace

Framebuffer::Framebuffer(const FramebufferSpecification& spec) : m_spec(spec) { invalidate(); }

Framebuffer::~Framebuffer() {
  if (m_rendererID) {
    glDeleteFramebuffers(1, &m_rendererID);
  }
}

void Framebuffer::invalidate() {
  if (m_rendererID) {
    glDeleteFramebuffers(1, &m_rendererID);
    m_colorAttachments.clear();
    m_depthAttachment = nullptr;
  }

  if (m_spec.swapChainTarget) {
    m_rendererID = 0;
    return;
  }

  glGenFramebuffers(1, &m_rendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

  uint32_t colorAttachmentIndex = 0;
  for (const auto& attachmentSpec : m_spec.attachments) {
    if (!IsDepthFormat(attachmentSpec.texture_format)) {
      auto texture = Texture::create2D(attachmentSpec.texture_format, m_spec.width, m_spec.height);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentIndex, GL_TEXTURE_2D,
                             texture->getRendererID(), 0);
      m_colorAttachments.push_back(texture);
      colorAttachmentIndex++;
    } else {
      m_depthAttachment = Texture::createDepthStencil(attachmentSpec.texture_format, m_spec.width, m_spec.height);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                             m_depthAttachment->getRendererID(), 0);
    }
  }

  if (m_colorAttachments.size() > 1) {
    assert(m_colorAttachments.size() <= 4 && "Only up to 4 color attachments are supported");
    GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(m_colorAttachments.size(), buffers);
  } else if (m_colorAttachments.empty()) {
    // Only depth-pass
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Nexus::Logger::error("Framebuffer is not complete!");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
  glViewport(0, 0, m_spec.width, m_spec.height);
}

void Framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::clear(Color color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Framebuffer::clearAttachment(uint32_t attachmentIndex, int value) {
  assert(attachmentIndex < m_colorAttachments.size());

  // auto& spec = m_colorAttachments[attachmentIndex]->getSpecification();
  // TODO(kobe): Make format configurable
  glClearBufferiv(GL_COLOR, attachmentIndex, &value);
}

void Framebuffer::resize(uint32_t width, uint32_t height) {
  if (width == 0 || height == 0 || width > 8192 || height > 8192) {
    Nexus::Logger::warn("Attempted to resize framebuffer to an invalid size: {}x{}", width, height);
    return;
  }

  if (m_spec.width == width && m_spec.height == height) {
    return;
  }

  m_spec.width = width;
  m_spec.height = height;

  invalidate();
}

int Framebuffer::readPixel(uint32_t attachmentIndex, int x, int y) {
  assert(attachmentIndex < m_colorAttachments.size());
  bind();
  glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
  int pixelData;
  // TODO(kobe): Make format configurable
  glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
  unbind();
  return pixelData;
}

uint32_t Framebuffer::getColorAttachmentRendererID(uint32_t index) const {
  assert(index < m_colorAttachments.size() && "Color attachment index out of range");
  return m_colorAttachments[index]->getRendererID();
}

uint32_t Framebuffer::getDepthAttachmentRendererID() const {
  assert(m_depthAttachment && "No depth attachment");
  return m_depthAttachment->getRendererID();
}

uint32_t Framebuffer::getDebugEntityIDTextureID() {
  m_debug_color_map.clear();
  m_debug_color_map[-1] = {0, 0, 0};
  int w = m_spec.width;
  int h = m_spec.height;

  // Read raw integer pixels from attachment 1
  std::vector<int> pixels(w * h);
  bind();
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glReadPixels(0, 0, w, h, GL_RED_INTEGER, GL_INT, pixels.data());
  unbind();

  // Simple hash-based color generation for any other ID
  auto id_to_color = [&](int id) -> std::tuple<uint8_t, uint8_t, uint8_t> {
    if (m_debug_color_map.contains(id)) return m_debug_color_map[id];
    // Spread IDs into visually distinct hues using golden ratio
    float hue = glm::fract(id * 0.618033f);
    // HSV to RGB with S=0.8, V=0.9
    float h = hue * 6.0f;
    float c = 0.8f * 0.9f;
    float x = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));
    glm::vec3 rgb{};
    if (h < 1)
      rgb = {c, x, 0};
    else if (h < 2)
      rgb = {x, c, 0};
    else if (h < 3)
      rgb = {0, c, x};
    else if (h < 4)
      rgb = {0, x, c};
    else if (h < 5)
      rgb = {x, 0, c};
    else
      rgb = {c, 0, x};
    auto result = std::make_tuple(static_cast<uint8_t>(rgb.r * 255), static_cast<uint8_t>(rgb.g * 255),
                                  static_cast<uint8_t>(rgb.b * 255));
    m_debug_color_map[id] = result;
    return result;
  };

  // Build RGBA8 output
  std::vector<uint8_t> rgba(w * h * 4);
  for (int i = 0; i < w * h; i++) {
    auto [r, g, b] = id_to_color(pixels[i]);
    rgba[i * 4 + 0] = r;
    rgba[i * 4 + 1] = g;
    rgba[i * 4 + 2] = b;
    rgba[i * 4 + 3] = 255;
  }

  // Upload to a persistent debug texture
  if (!m_debug_entity_id_texture) {
    glGenTextures(1, &m_debug_entity_id_texture);
  }
  glBindTexture(GL_TEXTURE_2D, m_debug_entity_id_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  return m_debug_entity_id_texture;
}