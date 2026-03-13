#pragma once

class Renderer;
class AssetManager;
class Entity;
class Scene;

struct RenderContext {
  Renderer& renderer;
  AssetManager& assets;
  Entity& selected_entity;
  Scene& scene;
};