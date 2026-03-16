#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "renderer/primitive.h"
class PatchPrimitive : public Primitive {
 public:
  explicit PatchPrimitive(int vertices_per_patch) : Primitive(GL_PATCHES), m_vertices_per_patch(vertices_per_patch) {}

  void update(const std::vector<glm::vec3>& points) {
    upload(reinterpret_cast<const float*>(points.data()), 3, points.size());
  }

  void draw() const override {
    glPatchParameteri(GL_PATCH_VERTICES, m_vertices_per_patch);
    Primitive::draw();
    glPatchParameteri(GL_PATCH_VERTICES, 3);
  }

 private:
  int m_vertices_per_patch;
};