#pragma once
#include <glad/gl.h>

#include "renderer/drawable.h"

class Primitive : public IDrawable {
 public:
  explicit Primitive(GLenum mode) : m_mode(mode) {}
  ~Primitive() override {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
  }

  Primitive(const Primitive&) = delete;
  Primitive& operator=(const Primitive&) = delete;

  void draw() const override {
    glBindVertexArray(m_vao);
    glDrawArrays(m_mode, 0, m_vertex_count);
    glBindVertexArray(0);
  }

 protected:
  void upload(const float* data, int floats_per_vertex, int vertex_count) {
    m_vertex_count = vertex_count;
    if (!m_vao) {
      glGenVertexArrays(1, &m_vao);
      glGenBuffers(1, &m_vbo);
    }
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * floats_per_vertex * sizeof(float), data, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, floats_per_vertex, GL_FLOAT, GL_FALSE, floats_per_vertex * sizeof(float), nullptr);
    glBindVertexArray(0);
  }

 private:
  GLenum m_mode;
  GLuint m_vao = 0, m_vbo = 0;
  int m_vertex_count = 0;
};