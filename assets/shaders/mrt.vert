#version 410 core
layout(location = 0) in vec3 aPos;

uniform mat4 u_model;
uniform mat4 u_proj_view;

void main() { gl_Position = u_proj_view * u_model * vec4(aPos, 1.0); }