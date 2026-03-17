#pragma once
class Scene;

class ILogicSystem {
 public:
  virtual void onUpdate(Scene& scene, float deltaTime) = 0;
  virtual ~ILogicSystem() = default;
};