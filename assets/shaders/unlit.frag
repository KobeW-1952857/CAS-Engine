#version 410 core

layout(location = 0) out vec4 out_color;

uniform vec4 u_color = vec4(0.8, 0.8, 0.8, 1.0);

void main() { out_color = u_color; }
