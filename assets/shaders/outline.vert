#version 410 core
#include "engine/uniforms.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 4) in vec3 a_Normal;

uniform float u_outline_width;

void main() {
  vec4 clip_pos = u_proj_view * u_model * vec4(a_Position, 1.0);

  vec4 clip_normal = u_proj_view * u_model * vec4(a_Normal, 0.0);

  vec2 screen_normal = normalize(clip_normal.xy);
  clip_pos.xy += screen_normal * (u_outline_width / u_viewport_size) * clip_pos.w;

  gl_Position = clip_pos;
}