#version 410 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 u_model;
uniform mat4 u_proj_view;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
  vec4 worldPos = u_model * vec4(aPosition, 1.0);

  vFragPos = worldPos.xyz;
  vNormal = mat3(transpose(inverse(u_model))) * aNormal;
  vTexCoord = aTexCoord;

  gl_Position = u_proj_view * worldPos;
}