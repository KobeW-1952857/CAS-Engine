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