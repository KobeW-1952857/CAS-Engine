#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "renderer/primitive.h"

class PointPrimitive : public Primitive {
 public:
  PointPrimitive() : Primitive(GL_POINTS) {}

  void update(const std::vector<glm::vec3>& points) {
    upload(reinterpret_cast<const float*>(points.data()), 3, static_cast<int>(points.size()));
  }
};