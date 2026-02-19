#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos = vec3(0.0, 0.0, 0.0);    // Position of your light
uniform vec3 lightColor = vec3(1.0, 1.0, 0.0);  // e.g., vec3(1.0, 1.0, 1.0)
uniform float intensity;                        // Intensity of the light
uniform bool isEmitter;                         // Is the planet an emitter?
uniform vec4 ourColor;                          // The planet's color
uniform sampler2D ourTexture;                   // The planet's texture
uniform bool hasTexture = false;                // Does the planet have a texture?

void main() {
  if (isEmitter) {
    FragColor = hasTexture ? texture(ourTexture, TexCoords) : ourColor;
    return;
  }

  // 1. Ambient - small constant light so shadows aren't pitch black
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  // 2. Diffuse (Lambertian)
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // 3. Attenuation (Inverse Square Law)
  float distance = length(lightPos - FragPos);
  float attenuation = intensity / (distance * distance);

  // Combine results
  vec4 result = vec4(ambient + diffuse * attenuation, 1.0);
  if (hasTexture) {
    vec4 texColor = texture(ourTexture, TexCoords);
    result = result * texColor;
    // result = texColor;
  } else {
    result = result * ourColor;
  }

  FragColor = result;
}