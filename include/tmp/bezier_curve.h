#pragma once

#include <glad/gl.h>

#include <array>

#include "renderer/shader.h"
#include "renderer/tesselation_shader.h"
#include "utils/utils.h"

// Start with a bezier curve
class BezierCurve {
 public:
  BezierCurve(const std::array<Point, 4>& control_points);
  BezierCurve(const Point& p0, const Point& p1, const Point& p2, const Point& p3);

  void render(TesselationShader& shader);
  void renderControlPoints(Shader& shader);
  void drawUI();

 private:
  void initOpenGLBuffers();

 private:
  std::array<Point, 4> m_control_points;
  GLuint m_vao, m_vbo;
};