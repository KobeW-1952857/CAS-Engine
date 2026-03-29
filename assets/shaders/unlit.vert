#version 410 core

#include "engine/uniforms.glsl"

layout(location = 0) in vec3 aPos;

void main() { gl_Position = u_proj_view * u_model * vec4(aPos, 1.0); }