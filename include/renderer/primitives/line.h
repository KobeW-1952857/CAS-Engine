#pragma once

#include <glm/glm.hpp>

#include "renderer/primitive.h"

class LinePrimitive : public Primitive {
 public:
  LinePrimitive() : Primitive(GL_LINES) {}

  void update(const glm::vec3& p0, const glm::vec3& p1) {
    float data[] = {p0.x, p0.y, p0.z, p1.x, p1.y, p1.z};
    upload(data, 3, 2);
  }
};