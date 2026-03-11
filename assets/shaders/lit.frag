#version 410 core
layout(location = 0) out vec4 out_color;
layout(location = 1) out int out_EntityID;

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;

// Camera
uniform vec3 u_view_pos;
uniform int u_entity_id;

uniform vec3 u_light_pos;
uniform vec3 uLightColor;
uniform vec3 uObjectColor;

// Lighting coefficients
uniform float uAmbientStrength;   // e.g. 0.1
uniform float uSpecularStrength;  // e.g. 0.5
uniform float uShininess;         // e.g. 32.0

void main() {
  vec3 norm = normalize(vNormal);
  vec3 lightDir = normalize(u_light_pos - vFragPos);
  vec3 viewDir = normalize(u_view_pos - vFragPos);
  vec3 halfVec = normalize(lightDir + viewDir);

  // Ambient
  vec3 ambient = uAmbientStrength * uLightColor;

  // Diffuse  (Lambertian)
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * uLightColor;

  // Specular  (Blinn-Phong)
  float spec = pow(max(dot(norm, halfVec), 0.0), uShininess);
  vec3 specular = uSpecularStrength * spec * uLightColor;

  vec3 result = (ambient + diffuse + specular) * uObjectColor;
  out_color = vec4(result, 1.0);
  out_EntityID = u_entity_id;
}