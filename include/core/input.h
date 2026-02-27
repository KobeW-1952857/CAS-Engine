#pragma once

#include <glm/glm.hpp>

class Input {
 public:
  static bool isKeyPressed(int keycode);

  static bool isMouseButtonPressed(int button);
  static glm::vec2 getMousePosition();
  static float getMouseX();
  static float getMouseY();

 private:
};