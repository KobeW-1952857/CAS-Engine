#pragma once

#include <glad/gl.h>

#include <memory>

#include "renderer/shader.h"

class FileSystem;

// Editor-only overlay that draws an infinite XZ-plane grid with coloured axis
// lines (X=red, Z=blue).  Rendered after the scene colour pass so it is depth-
// tested against scene geometry but does not write to the depth buffer itself.
class GridOverlay {
 public:
  explicit GridOverlay(FileSystem& fs);
  ~GridOverlay();

  GridOverlay(const GridOverlay&) = delete;
  GridOverlay& operator=(const GridOverlay&) = delete;

  // Call while the framebuffer is still bound, after the scene colour pass.
  void render();

  bool enabled = true;
  float scale = 100.0f;

 private:
  std::shared_ptr<Shader> m_shader;
  GLuint m_vao = 0;
};