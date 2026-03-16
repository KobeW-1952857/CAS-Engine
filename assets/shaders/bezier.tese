#version 410 core
layout(isolines, equal_spacing, ccw) in;

void main() {
  float t = gl_TessCoord.x;

  // The 4 control points from the TCS
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  vec4 p3 = gl_in[3].gl_Position;

  // Bernstein Polynomials / Bézier Formula
  float it = 1.0 - t;
  float b0 = it * it * it;
  float b1 = 3.0 * it * it * t;
  float b2 = 3.0 * it * t * t;
  float b3 = t * t * t;

  gl_Position = b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;
}