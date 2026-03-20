#version 410 core
#include "engine/uniforms.glsl"

in  vec3 v_world_pos;
out vec4 FragColor;

// Returns 1.0 on a grid line, 0.0 in the cell interior, anti-aliased via screen-space
// derivatives.  cell_size is in world units.
float gridLine(vec2 pos, float cell_size) {
    vec2 deriv = fwidth(pos);
    vec2 grid  = abs(fract(pos / cell_size - 0.5) - 0.5) / deriv;
    return 1.0 - clamp(min(grid.x, grid.y), 0.0, 1.0);
}

// Returns 1.0 at the axis line (coord == 0), falls off over ~2 screen pixels.
float axisLine(float coord) {
    return 1.0 - smoothstep(0.0, fwidth(coord) * 2.0, abs(coord));
}

void main() {
    vec2  uv   = v_world_pos.xz;
    float dist = length(uv - u_cam_pos.xz);

    // Fade the grid out beyond a comfortable viewing distance.
    float fade = 1.0 - smoothstep(40.0, 100.0, dist);
    if (fade < 0.005) discard;

    // Two grid scales: 1-unit (fine) and 10-unit (coarse).
    float fine_grid   = gridLine(uv,  1.0);
    float coarse_grid = gridLine(uv, 10.0);
    float grid        = max(fine_grid * 0.25, coarse_grid * 0.6);

    // World-axis lines.  X axis runs along z = 0; Z axis runs along x = 0.
    float x_axis = axisLine(v_world_pos.z);
    float z_axis = axisLine(v_world_pos.x);

    // Compose colour: neutral grid, then tint the axis lines.
    vec3 color = vec3(0.4) * grid;
    color = mix(color, vec3(0.85, 0.2, 0.2), x_axis);  // X axis: red
    color = mix(color, vec3(0.2, 0.2, 0.85), z_axis);  // Z axis: blue

    float alpha = max(grid, max(x_axis, z_axis)) * fade;
    if (alpha < 0.005) discard;

    FragColor = vec4(color, alpha);
}
