#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
#include "engine/uniforms.glsl"

uniform float u_radius;

out vec2 v_uv;

void main() {
  vec4 center = gl_in[0].gl_Position;

  // Emit the vertices of the billboard quad
  v_uv = vec2(-1.0, -1.0);
  gl_Position = center + vec4(v_uv * u_radius, 0.0, 0.0);
  EmitVertex();

  v_uv = vec2(1.0, -1.0);
  gl_Position = center + vec4(v_uv * u_radius, 0.0, 0.0);
  EmitVertex();

  v_uv = vec2(-1.0, 1.0);
  gl_Position = center + vec4(v_uv * u_radius, 0.0, 0.0);
  EmitVertex();

  v_uv = vec2(1.0, 1.0);
  gl_Position = center + vec4(v_uv * u_radius, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}