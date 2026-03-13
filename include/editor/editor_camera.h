#pragma once

#include "glm/glm.hpp"

class EditorCamera {
 public:
  EditorCamera() { updateProjection(); };
  EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip);

  void onUpdate(float dt, bool allow_keyboard);
  void focusEntity(const glm::vec3& position, const float distance);
  void setViewportSize(float width, float height);

  const glm::mat4& getViewMatrix() const { return m_view; }
  const glm::mat4& getProjectionMatrix() const { return m_projection; }
  glm::mat4 getViewProjectionMatrix() const { return m_projection * m_view; }

  glm::vec3 getUpDirection() const;
  glm::vec3 getRightDirection() const;
  glm::vec3 getForwardDirection() const;
  const glm::vec3& getPosition() const { return m_position; }
  glm::quat getOrientation() const;
  float getPitch() const { return m_pitch; }
  float getYaw() const { return m_yaw; }

 private:
  void updateProjection();
  void updateView();

  void freeFly(const glm::vec2& delta, float dt, bool allow_keyboard);
  void mousePan(const glm::vec2& delta);
  void mouseRotate(const glm::vec2& delta);
  void mouseZoom(float delta);

  float panSpeed() const;
  float rotationSpeed() const;
  float zoomSpeed() const;

 private:
  float m_fov = 45.0f, m_aspect_ratio = 16.0f / 9.0f, m_near_clip = 0.1f, m_far_clip = 1000.0f;

  glm::mat4 m_projection = glm::mat4(1.0f), m_view = glm::mat4(1.0f);
  glm::vec3 m_position = {0, 0, 0}, m_focal_point = {0, 0, 0};

  glm::vec2 m_initial_mouse_position = {0, 0};

  float m_distance = 10.0f;
  float m_pitch = 0.0f, m_yaw = 0.0f;

  float m_viewport_width = 1280, m_viewport_height = 720;

  float m_normal_speed = 5.0f, m_fast_speed = 15.0f;
};