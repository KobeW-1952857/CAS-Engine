#pragma once

#include <concepts>
#include <string_view>
#include <tuple>

#include "scene/components.h"

class Entity;
class AppContext;

template <typename T>
concept Component =
    requires(YAML::Emitter& out, const T& c, const YAML::Node& node, Entity entity, T& mut, AppContext& ctx) {
      { T::name } -> std::convertible_to<std::string_view>;
      { T::user_addable } -> std::convertible_to<bool>;
      { T::user_removable } -> std::convertible_to<bool>;
      { T::user_editable } -> std::convertible_to<bool>;

      { T::serialize(out, c) } -> std::same_as<void>;
      { T::deserialize(node, entity) } -> std::same_as<void>;
      { T::drawUI(mut, ctx) } -> std::same_as<void>;
    };

using AllComponentTypes =
    std::tuple<IDComponent, TagComponent, ParentComponent, ChildrenComponent, TransformComponent, MeshComponent,
               MaterialComponent, LineComponent, BezierComponent, LineFollowerComponent, LaticeComponent>;

template <typename T>
concept CurveConcept = requires(const T& a, float t) {
  { a.evaluate(t) } -> std::same_as<glm::vec3>;
  { a.length() } -> std::same_as<float>;
};
using AllCurveTypes = std::tuple<LineComponent, BezierComponent>;

namespace detail {
template <typename T>
constexpr bool validateComponent() {
  static_assert(requires { T::name; }, "Component must have static 'name' member");
  static_assert(
      requires(YAML::Emitter& out, const T& c) { T::serialize(out, c); },
      "Component must have: static void serialize(YAML::Emitter&, const T&)");
  static_assert(
      requires(const YAML::Node& n, Entity e) { T::deserialize(n, e); },
      "Component must have: static void deserialize(const YAML::Node&, Entity)");
  static_assert(
      requires(T& c, AppContext& ctx) { T::drawUI(c, ctx); },
      "Component must have: static void drawUI(T&, AppContext&)");
  return true;
}

template <typename Tuple>
struct AllSatisfyComponent;

template <typename... Ts>
struct AllSatisfyComponent<std::tuple<Ts...>> {
  static constexpr bool value = (Component<Ts> && ...);
};
}  // namespace detail

template <typename F>
void forEachComponentType(F&& f) {
  std::apply([&](auto... dummy) { (f.template operator()<decltype(dummy)>(), ...); }, AllComponentTypes{});
}

static_assert([]<typename... Ts>(std::type_identity<std::tuple<Ts...>>) {
  return (detail::validateComponent<Ts>() && ...);
}(std::type_identity<AllComponentTypes>{}));

static_assert(detail::AllSatisfyComponent<AllComponentTypes>::value,
              "All types in AllComponentTypes must satisfy the Component concept");