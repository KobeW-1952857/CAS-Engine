#include "scene/systems/line_follower_system.h"

#include <variant>

#include "scene/component_type_list.h"
#include "scene/components.h"
#include "scene/scene.h"

void LineFollowerSystem::onUpdate(Scene& scene, float dt) {
  scene.forEachEntity<LineFollowerComponent>([&](Entity e) {
    auto& follower = e.getComponent<LineFollowerComponent>();

    Entity curve_entity = scene.getEntity(follower.curve_entity);

    if (auto v = curve_entity.tryGetOneOf<AllCurveTypes>()) {
      std::visit(
          [&](auto& ref_wrapper) {
            const CurveConcept auto& curve = ref_wrapper.get();
            float length = curve.length();
            if (length == 0.0f) return;

            follower.t += (follower.speed * dt) / length;
            if (follower.t > 1.0f) {
              if (follower.loop) {
                follower.t = std::fmod(follower.t, 1.0f);
              } else {
                follower.t = 1.0f;
              }
            }

            glm::vec3 new_position = curve.evaluate(follower.t);
            auto& transform = e.getComponent<TransformComponent>();
            transform.translation = new_position;
          },
          *v);
    }
  });
}