#version 410 core

layout(location = 0) out int out_EntityID;

uniform int u_EntityID;

void main() { out_EntityID = u_EntityID; }