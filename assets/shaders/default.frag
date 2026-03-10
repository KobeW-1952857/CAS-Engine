#version 410 core

out vec4 FragColor;

uniform vec4 u_color = vec4(0.8, 0.8, 0.8, 1.0);

void main() { FragColor = u_color; }
