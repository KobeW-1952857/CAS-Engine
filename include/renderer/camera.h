#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#include "utils/utils.h"

namespace Nexus {
class Window;
}
class Camera {
 public:
  Camera(Point position, float fov);

  void addControls(Nexus::Window& window);
  void processInput(Nexus::Window& window);

  Point getPosition() const;
  void setPosition(Point position);

  float* getPositionPtr();

  Direction getFront() const;
  void setFront(Direction front);
  float* getFrontPtr();

  Direction getUp() const;

  void move(Direction direction);
  void rotate(glm::vec2 delta);

  glm::mat4 getView() const;
  glm::mat4 getProjection(float width, float height) const;

  void updateVectors();

 private:
  Point m_position;
  // Camera direction unit vectors
  Direction m_front;
  Direction m_up;
  Direction m_right;

  float m_yaw = -90.0f;
  float m_pitch = 0.0f;

  float m_fov = 90.0f;
};