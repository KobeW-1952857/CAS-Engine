#include "bezier_curve.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

BezierCurve::BezierCurve(const std::array<Point, 4>& control_points) : m_control_points(control_points) {
  initOpenGLBuffers();
}
BezierCurve::BezierCurve(const Point& p0, const Point& p1, const Point& p2, const Point& p3)
    : m_control_points({p0, p1, p2, p3}) {
  initOpenGLBuffers();
}

void BezierCurve::render(TesselationShader& shader) {
  shader.use();
  glPatchParameteri(GL_PATCH_VERTICES, 4);

  glBindVertexArray(m_vao);
  glDrawArrays(GL_PATCHES, 0, 4);
}

void BezierCurve::renderControlPoints(Shader& shader) {
  shader.use();

  glBindVertexArray(m_vao);
  glDrawArrays(GL_POINTS, 0, 4);
}

void BezierCurve::drawUI() {
  ImGui::Text("Control points");
  bool changed = false;
  changed |= ImGui::InputFloat3("P0", glm::value_ptr(m_control_points[0]));
  changed |= ImGui::InputFloat3("P1", glm::value_ptr(m_control_points[1]));
  changed |= ImGui::InputFloat3("P2", glm::value_ptr(m_control_points[2]));
  changed |= ImGui::InputFloat3("P3", glm::value_ptr(m_control_points[3]));

  if (changed) {
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_control_points), m_control_points.data());
  }
}

void BezierCurve::initOpenGLBuffers() {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_control_points), m_control_points.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
}