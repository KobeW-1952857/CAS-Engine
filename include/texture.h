#pragma once

#include <string>

class Texture {
 public:
  Texture() = default;
  Texture(const std::string& path);
  ~Texture();

  void load(const std::string& path);

  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  void bind(unsigned int unit = 0) const;
  unsigned int getRendererID() const { return m_rendererID; }
  void unbind() const;

 private:
  unsigned int m_rendererID = 0;
  std::string m_filePath;
  int m_width, m_height, m_bpp;
};