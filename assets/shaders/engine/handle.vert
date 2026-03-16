#version 410 core

#include "engine/uniforms.glsl"
layout(location = 0) in vec3 aPosition;
void main() { gl_Position = u_proj_view * vec4(aPosition, 1.0); }