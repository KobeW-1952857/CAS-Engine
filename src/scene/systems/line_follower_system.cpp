#include "scene/systems/line_follower_system.h"

#include <variant>

#include "scene/component_type_list.h"
#include "scene/components.h"
#include "scene/scene.h"

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

            float t = curve.tFromArcLength(follower.arc_length);

            glm::vec3 new_position = curve.evaluate(t);
            auto& transform = e.getComponent<TransformComponent>();
            transform.translation = new_position;
          },
          *v);
    }
  });
}