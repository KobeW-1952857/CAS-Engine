#pragma once

#include <entt.hpp>
#include <memory>

#include "scene/scene.h"

class Entity {
 public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}

  template <typename T, typename... Args>
  T& addComponent(Args&&... args) {
    T& component = m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
    return component;
  }

  template <typename T>
  T& getComponent() const {
    return m_scene->m_registry.get<T>(m_handle);
  }
  template <typename T>
  T* tryGetComponent() const {
    return m_scene->m_registry.try_get<T>(m_handle);
  }

  template <typename T>
  bool hasComponent() {
    return m_scene->m_registry.all_of<T>(m_handle);
  }

  operator bool() const { return m_handle != entt::null; }
  operator entt::entity() const { return m_handle; }
  operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

  bool operator==(const Entity& other) const { return m_handle == other.m_handle; }
  bool operator!=(const Entity& other) const { return !(*this == other); }

 private:
  entt::entity m_handle{entt::null};
  Scene* m_scene;
};