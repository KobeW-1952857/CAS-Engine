#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION

#include <Nexus/Window/GLFWWindow.h>
#include <stb_image.h>

#include <iostream>

Texture::Texture(const std::string& path) : m_rendererID(0), m_filePath(path), m_width(0), m_height(0), m_bpp(0) {
  load(path);
}

void Texture::load(const std::string& path) {
  if (m_rendererID != 0) {
    glDeleteTextures(1, &m_rendererID);
  }

  m_filePath = path;

  stbi_set_flip_vertically_on_load(1);
  unsigned char* localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);

  glGenTextures(1, &m_rendererID);
  glBindTexture(GL_TEXTURE_2D, m_rendererID);

  // Standard scaling/wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (localBuffer) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(localBuffer);
  } else {
    std::cout << "Failed to load texture: " << path << std::endl;
  }
}

Texture::~Texture() {
  if (m_rendererID != 0) glDeleteTextures(1, &m_rendererID);
}

Texture::Texture(Texture&& other) noexcept
    : m_rendererID(other.m_rendererID),
      m_filePath(std::move(other.m_filePath)),
      m_width(other.m_width),
      m_height(other.m_height),
      m_bpp(other.m_bpp) {
  other.m_rendererID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
  if (this != &other) {
    if (m_rendererID != 0) {
      glDeleteTextures(1, &m_rendererID);
    }

    m_rendererID = other.m_rendererID;
    m_filePath = std::move(other.m_filePath);
    m_width = other.m_width;
    m_height = other.m_height;
    m_bpp = other.m_bpp;

    other.m_rendererID = 0;
  }
  return *this;
}

void Texture::bind(unsigned int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, m_rendererID);
}

void Texture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }