#pragma once

#include <string>
#include <vector>

#include "Nexus/Window/GLFWWindow.h"
#include "core/asset.h"
#include "utils/utils.h"

struct Vertex {
  Point position;
  Direction normal;
  TextureCoord texCoord;
  Color color;
};

class Mesh : public Asset {
 public:
  Mesh() { type = AssetType::Mesh; }
  Mesh(std::string filepath);
  Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

  void render();

  const std::string& getName() const { return m_name; }
  const std::string& getFilePath() const { return m_filepath; }

  operator bool() const { return m_initialized; }

 private:
  void initOpenGLBuffers();

 private:
  std::string m_name;
  std::string m_filepath;
  bool m_initialized = false;
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  GLuint m_vao, m_vbo, m_ebo;
};