#version 410 core

layout(location = 1) out int out_entity_id;

uniform int u_entity_id;

void main() { out_entity_id = u_entity_id; }