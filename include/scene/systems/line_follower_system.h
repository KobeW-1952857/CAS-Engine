#pragma once

#include "scene/logic_system.h"
class LineFollowerSystem : public ILogicSystem {
 public:
  void onUpdate(Scene& scene, float deltaTime) override;
};