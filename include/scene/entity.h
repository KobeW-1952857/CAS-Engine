#pragma once

#include <entt.hpp>
#include <functional>
#include <optional>
#include <tuple>
#include <variant>

template <typename... Ts>
using OneOf = std::optional<std::variant<std::reference_wrapper<Ts>...>>;

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

  template <typename... Ts>
    requires(sizeof...(Ts) != 1 || !entt::is_tuple_v<std::tuple_element_t<0, std::tuple<Ts...>>>)
  OneOf<Ts...> tryGetOneOf() const {
    OneOf<Ts...> result;
    ((!result && m_registry->try_get<Ts>(m_handle) ? (result = std::ref(*m_registry->try_get<Ts>(m_handle)), true)
                                                   : false) ||
     ...);
    return result;
  }

  template <typename T>
    requires entt::is_tuple<T>::value
  auto tryGetOneOf() const {
    return [this]<typename... Ts>(std::tuple<Ts...>*) { return tryGetOneOf<Ts...>(); }((T*)nullptr);
  }

  template <typename T>
  bool hasComponent() const {
    return m_registry->all_of<T>(m_handle);
  }

  template <typename T>
  void removeComponent() {
    m_registry->remove<T>(m_handle);
  }

  operator bool() const { return m_handle != entt::null; }
  operator entt::entity() const { return m_handle; }
  operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

  bool operator==(const Entity& other) const { return m_handle == other.m_handle; }

 private:
  entt::entity m_handle{entt::null};
  entt::registry* m_registry;
};