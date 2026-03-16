#version 410 core

#include "engine/uniforms.glsl"

in vec2 v_uv;
uniform vec3 u_color;
out vec4 out_color;

void main() {
  float dist = dot(v_uv, v_uv);
  if (dist > 1.0) discard;

  vec3 normal = normalize(vec3(v_uv, sqrt(1.0 - dist)));
  vec3 light = normalize(vec3(1, 2, 1));
  float diffuse = max(dot(normal, light), 0.0) * 0.7 + 0.3;
  out_color = vec4(u_color * diffuse, 1.0);
}