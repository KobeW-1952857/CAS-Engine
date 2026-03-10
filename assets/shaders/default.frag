#version 410 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out int out_EntityID;

uniform vec4 u_color = vec4(0.8, 0.8, 0.8, 1.0);
uniform int u_entity_id;

void main() {
  out_EntityID = u_entity_id;
  out_color = u_color;
}
