#version 410 core
layout(vertices = 4) out;  // 4 control points per patch

void main() {
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  if (gl_InvocationID == 0) {
    gl_TessLevelOuter[0] = 1.0;   // Number of segments in one direction
    gl_TessLevelOuter[1] = 64.0;  // How "smooth" the curve is
  }
}