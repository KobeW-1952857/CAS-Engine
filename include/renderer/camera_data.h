#pragma once

#include <glm/glm.hpp>
struct CameraData {
  glm::vec2 viewport_size;
  glm::vec3 cam_pos;
  glm::mat4 view_proj;
  glm::mat4 projection;
  glm::mat4 view;
};