#pragma once

#include <entt.hpp>

class Entity {
 public:
  Entity() = default;
  Entity(entt::entity handle, entt::registry* registry) : m_handle(handle), m_registry(registry) {}

  template <typename T, typename... Args>
  T& addComponent(Args&&... args) {
    T& component = m_registry->emplace<T>(m_handle, std::forward<Args>(args)...);
    return component;
  }

  template <typename T>
  T& getComponent() const {
    return m_registry->get<T>(m_handle);
  }
  template <typename T>
  T* tryGetComponent() const {
    return m_registry->try_get<T>(m_handle);
  }

  template <typename T>
  bool hasComponent() {
    return m_registry->all_of<T>(m_handle);
  }

  operator bool() const { return m_handle != entt::null; }
  operator entt::entity() const { return m_handle; }
  operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

  bool operator==(const Entity& other) const { return m_handle == other.m_handle; }

 private:
  entt::entity m_handle{entt::null};
  entt::registry* m_registry;
};