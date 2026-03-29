#include "scene/systems/line_follower_system.h"

#include <variant>

#include "scene/component_type_list.h"
#include "scene/components.h"
#include "scene/scene.h"

float evaluate_easing(LineFollowerComponent lc, float t) {
  switch (lc.easing) {
    case LineFollowerComponent::Easing::Linear:
      return t;
    case LineFollowerComponent::Easing::EaseIn:
      return t * t;
    case LineFollowerComponent::Easing::EaseOut:
      return t * (2 - t);
    case LineFollowerComponent::Easing::EaseInOut:
      return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
    default:  // Custom or COUNT
      // Bernstein polynomial for cubic bezier with control points (0,0), (c1.x, c1.y), (c2.x, c2.y), (1,1)
      float u = 1 - t;
      float tt = t * t;
      float uu = u * u;
      float uuu = uu * u;
      float ttt = tt * t;
      float c1x = lc.control_points[0].x;
      float c1y = lc.control_points[0].y;
      float c2x = lc.control_points[1].x;
      float c2y = lc.control_points[1].y;
      return uuu * 0 + 3 * uu * t * c1y + 3 * u * tt * c2y + ttt * 1;
  }
}

void LineFollowerSystem::onUpdate(Scene& scene, float dt) {
  scene.forEachEntity<LineFollowerComponent>([&](Entity e) {
    auto& follower = e.getComponent<LineFollowerComponent>();

    Entity curve_entity = scene.getEntity(follower.curve_entity);

    // !TEMP: This is a bit hacky, but it allows us to use the same system for both line and bezier followers without
    // ! having to duplicate code or use inheritance.We can use std::variant to store either a LineComponent or a
    // ! BezierComponent, and then use std::visit to call the appropriate functions on them.
    if (auto v = curve_entity.tryGetOneOf<AllCurveTypes>()) {
      std::visit(
          [&](auto& ref_wrapper) {
            const CurveConcept auto& curve = ref_wrapper.get();
            float length = curve.length();
            if (length == 0.0f) return;

            follower.arc_length += follower.speed * dt;
            if (follower.loop)
              follower.arc_length = std::fmod(follower.arc_length, length);
            else
              follower.arc_length = std::min(follower.arc_length, length);

            float t_linear = follower.arc_length / length;
            float t_eased = evaluate_easing(follower, t_linear);
            float t = curve.tFromArcLength(t_eased * length);

            glm::vec3 new_position = curve.evaluate(t);
            auto& transform = e.getComponent<TransformComponent>();
            transform.translation = new_position;
          },
          *v);
    }
  });
}