#pragma once

#include <Nexus/Log.h>

#include <glm/glm.hpp>

#include "renderer/primitive.h"

class LinePrimitive : public Primitive {
 public:
  LinePrimitive() : Primitive(GL_LINES) {}

  void update(const glm::vec3& p0, const glm::vec3& p1) {
    float data[] = {p0.x, p0.y, p0.z, p1.x, p1.y, p1.z};
    upload(data, 3, 2);
  }
  void update(const std::vector<glm::vec3>& points) {
    if (points.size() % 2 != 0) {
      Nexus::Logger::error("LinePrimitive::update vector size should be multiple of 2, got {}", points.size());
      return;
    }

    std::vector<float> data;
    data.reserve(points.size() * 3);
    for (const auto& p : points) {
      data.push_back(p.x);
      data.push_back(p.y);
      data.push_back(p.z);
    }
    upload(data.data(), 3, static_cast<int>(points.size()));
  }
};

class LineStripPrimitive : public Primitive {
 public:
  LineStripPrimitive() : Primitive(GL_LINE_STRIP) {}

  void update(const std::vector<glm::vec3>& points) {
    std::vector<float> data;
    data.reserve(points.size() * 3);
    for (const auto& p : points) {
      data.push_back(p.x);
      data.push_back(p.y);
      data.push_back(p.z);
    }
    upload(data.data(), 3, static_cast<int>(points.size()));
  }
};