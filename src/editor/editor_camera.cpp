#include "editor/editor_camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GLFW/glfw3.h"
#include "Nexus/Log.h"
#include "core/input.h"

EditorCamera::EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip)
    : m_fov(fov), m_aspect_ratio(aspect_ratio), m_near_clip(near_clip), m_far_clip(far_clip) {
  updateProjection();
}

void EditorCamera::onUpdate(float dt) {
  glm::vec2 mouse = Input::getMousePosition();
  glm::vec2 delta = (mouse - m_initial_mouse_position) * 0.003f;
  m_initial_mouse_position = mouse;

  if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    freeFly(delta, dt);
  } else if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
    mousePan(delta);
  } else if (Input::isKeyPressed(GLFW_KEY_LEFT_ALT) && Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    mouseRotate(delta);
  }

  // TODO: zoom
  updateView();
}

void EditorCamera::updateProjection() {
  m_aspect_ratio = m_viewport_width / m_viewport_height;
  m_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near_clip, m_far_clip);
}

void EditorCamera::updateView() {
  m_position = m_focal_point - getForwardDirection() * m_distance;

  glm::quat orientation = getOrientation();
  m_view = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
  m_view = glm::inverse(m_view);
}

void EditorCamera::setViewportSize(float width, float height) {
  // if (width == 0.0f || height == 0.0f) return;
  if (width == m_viewport_width && height == m_viewport_height) return;
  m_viewport_width = width;
  m_viewport_height = height;
  updateProjection();
}

void EditorCamera::freeFly(const glm::vec2& delta, float dt) {
  mouseRotate(delta);

  float current_speed = Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT) ? m_fast_speed : m_normal_speed;
  float velocity = current_speed * dt;

  glm::vec3 movement(0.0f);
  if (Input::isKeyPressed(GLFW_KEY_W)) movement += getForwardDirection();
  if (Input::isKeyPressed(GLFW_KEY_S)) movement -= getForwardDirection();
  if (Input::isKeyPressed(GLFW_KEY_A)) movement -= getRightDirection();
  if (Input::isKeyPressed(GLFW_KEY_D)) movement += getRightDirection();
  if (Input::isKeyPressed(GLFW_KEY_E)) movement += getUpDirection();
  if (Input::isKeyPressed(GLFW_KEY_Q)) movement -= getUpDirection();

  if (glm::length(movement) > 0.0f) {
    m_focal_point += glm::normalize(movement) * velocity;
  }
}

void EditorCamera::mousePan(const glm::vec2& delta) {
  auto speed = panSpeed();
  m_focal_point += -getRightDirection() * delta.x * speed * m_distance;
  m_focal_point += getUpDirection() * delta.y * speed * m_distance;
}

void EditorCamera::mouseRotate(const glm::vec2& delta) {
  float yawSign = getUpDirection().y < 0 ? -1.0f : 1.0f;
  m_yaw += yawSign * delta.x * rotationSpeed();
  m_pitch += delta.y * rotationSpeed();
}

void EditorCamera::mouseZoom(float delta) {
  m_distance -= delta * zoomSpeed();
  if (m_distance < 1.0f) {
    m_focal_point += getForwardDirection();
    m_distance = 1.0f;
  }
}

glm::vec3 EditorCamera::getUpDirection() const { return glm::rotate(getOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }
glm::vec3 EditorCamera::getRightDirection() const { return glm::rotate(getOrientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }
glm::vec3 EditorCamera::getForwardDirection() const {
  return glm::rotate(getOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::quat EditorCamera::getOrientation() const { return glm::quat(glm::vec3(-m_pitch, -m_yaw, 0.0f)); }

float EditorCamera::panSpeed() const { return 0.0366f * (m_viewport_width / 1000.0f); }
float EditorCamera::rotationSpeed() const { return 0.8f; }
float EditorCamera::zoomSpeed() const { return m_distance * 0.2f; }