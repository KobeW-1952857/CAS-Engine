#version 410 core
#include "engine/uniforms.glsl"

// Four corners of a large XZ-plane quad, assembled via gl_VertexID (no VAO data needed).
const vec2 CORNERS[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

uniform float u_scale;  // half-extent of the grid plane in world units

out vec3 v_world_pos;

void main() {
    vec2 c      = CORNERS[gl_VertexID];
    v_world_pos = vec3(c.x * u_scale, 0.0, c.y * u_scale);
    gl_Position = u_proj_view * vec4(v_world_pos, 1.0);
}
