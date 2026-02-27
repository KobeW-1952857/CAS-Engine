#include "core/input.h"

#include <GLFW/glfw3.h>

bool Input::isKeyPressed(int keycode) {
  GLFWwindow* window = glfwGetCurrentContext();
  if (!window) return false;

  int state = glfwGetKey(window, keycode);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int button) {
  GLFWwindow* window = glfwGetCurrentContext();
  if (!window) return false;

  int state = glfwGetMouseButton(window, button);
  return state == GLFW_PRESS;
}

glm::vec2 Input::getMousePosition() {
  GLFWwindow* window = glfwGetCurrentContext();
  if (!window) return glm::vec2(0, 0);

  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return glm::vec2(x, y);
}

float Input::getMouseX() { return getMousePosition().x; }

float Input::getMouseY() { return getMousePosition().y; }