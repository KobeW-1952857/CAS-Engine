#version 410 core

layout(location = 0) out vec4 out_Color;
layout(location = 1) out int out_EntityID;

// in vec3 v_FragPos;
// in vec3 v_Normal;
// in vec2 v_TexCoord;
// in vec4 v_Color;

// Uniforms
uniform int u_entity_id;
uniform vec3 u_color;

void main() {
  // // --- Your existing rendering logic to calculate finalColor ---
  // // For example, a simple diffuse lighting model:
  // vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
  // float diff = max(dot(normalize(v_Normal), lightDir), 0.0);
  // vec3 diffuse = diff * vec3(1.0);
  // vec3 finalColor = (0.1 + diffuse) * u_Color * v_Color.rgb;
  // // --- End of existing logic ---

  // Output final color to the first attachment
  out_Color = vec4(u_color, 1.0);

  // Output the entity ID to the second attachment
  out_EntityID = u_entity_id;
}
