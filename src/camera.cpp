#include "camera.h"

#include <Nexus/Window/GLFWWindow.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include "utils.h"

Camera::Camera(Point position, float fov) : m_position(position), m_fov(fov) { updateVectors(); }

void Camera::addControls(Nexus::Window& window) {
  auto* native_window = static_cast<GLFWwindow*>(window.getNativeWindow());

  window.onMouseMove([this, native_window, &window](double xpos, double ypos) -> bool {
    static bool firstMouse = true;
    static glm::vec2 last_pos{};
    static float sensitivity = 0.1f;

    if (glfwGetMouseButton(native_window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
      firstMouse = true;
      return false;
    }

    if (firstMouse) {
      last_pos = {xpos, ypos};
      firstMouse = false;
    }

    glm::vec2 delta = {xpos - last_pos.x, last_pos.y - ypos};
    last_pos = {xpos, ypos};

    this->rotate(delta * sensitivity);

    return true;
  });

  window.onMouseButton([&window, native_window](int button, int action, int mods) -> bool {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        glfwSetInputMode(native_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      } else if (action == GLFW_RELEASE) {
        glfwSetInputMode(native_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
      return true;
    }
    return false;
  });
}

Point Camera::getPosition() const { return m_position; }

void Camera::setPosition(Point position) { m_position = position; }

float* Camera::getPositionPtr() { return glm::value_ptr(m_position); }

Direction Camera::getFront() const { return m_front; }

void Camera::setFront(Direction front) {
  m_pitch = glm::atan(front.y / glm::sqrt(front.x * front.x + front.z * front.z));
  m_yaw = glm::atan(front.x / front.z);
  updateVectors();
}

float* Camera::getFrontPtr() { return glm::value_ptr(m_front); }

Direction Camera::getUp() const { return m_up; }

void Camera::move(Direction direction) {
  m_position += m_front * direction.z + m_right * direction.x + m_up * direction.y;
}

void Camera::processInput(Nexus::Window& window) {
  static float speed = 0.05f;
  auto* native_window = static_cast<GLFWwindow*>(window.getNativeWindow());

  if (glfwGetKey(native_window, GLFW_KEY_W) == GLFW_PRESS) move({0, 0, speed});   // Forward (Z)
  if (glfwGetKey(native_window, GLFW_KEY_S) == GLFW_PRESS) move({0, 0, -speed});  // Backward
  if (glfwGetKey(native_window, GLFW_KEY_A) == GLFW_PRESS) move({-speed, 0, 0});  // Left (X)
  if (glfwGetKey(native_window, GLFW_KEY_D) == GLFW_PRESS) move({speed, 0, 0});   // Right
  if (glfwGetKey(native_window, GLFW_KEY_E) == GLFW_PRESS) move({0, speed, 0});   // Up (Y)
  if (glfwGetKey(native_window, GLFW_KEY_Q) == GLFW_PRESS) move({0, -speed, 0});  // Down
}

void Camera::rotate(glm::vec2 delta) {
  m_yaw += delta.x;
  m_pitch += delta.y;

  if (m_pitch > 89.0) {
    m_pitch = 89.0;
  }
  if (m_pitch < -89.0) {
    m_pitch = -89.0;
  }

  updateVectors();
}

glm::mat4 Camera::getView() const { return glm::lookAt(m_position, m_position + m_front, m_up); }

glm::mat4 Camera::getProjection(float width, float height) const {
  return glm::perspective(glm::radians(m_fov), width / height, 0.1f, 10000.0f);
}

void Camera::updateVectors() {
  Direction front;
  front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  front.y = sin(glm::radians(m_pitch));
  front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  m_front = glm::normalize(front);

  Direction worldUp = (std::abs(m_front.y) > 0.99f) ? Direction(0, 0, -1) : Direction(0, 1, 0);

  m_right = glm::normalize(glm::cross(m_front, worldUp));
  m_up = glm::normalize(glm::cross(m_right, m_front));
}
