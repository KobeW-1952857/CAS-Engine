#pragma once

#include "core/asset_manager.h"
#include "core/project.h"
#include "core/simulation_clock.h"
#include "renderer/renderer.h"
#include "utils/filesystem.h"

struct AppContext {
  FileSystem filesystem;
  AssetManager assets{filesystem};
  Renderer renderer;
  Project project{filesystem, assets};
  SimulationClock clock;
  Scene* active_scene = nullptr;

  // Non-copyable and non-movable: members hold references to each other.
  AppContext() = default;
  AppContext(const AppContext&) = delete;
  AppContext& operator=(const AppContext&) = delete;
  AppContext(AppContext&&) = delete;
  AppContext& operator=(AppContext&&) = delete;
};