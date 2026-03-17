#pragma once

#include <glm/glm.hpp>

struct ICurve {
  virtual glm::vec3 evaluate(float t) const = 0;
  virtual float length() const = 0;
  virtual ~ICurve() = default;
};