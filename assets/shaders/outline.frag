#version 410 core
layout(location = 0) out vec4 out_Color;

uniform vec3 u_outline_color;

void main() { out_Color = vec4(u_outline_color, 1.0); }