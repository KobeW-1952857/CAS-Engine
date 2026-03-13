#pragma once

#include "core/asset_manager.h"
#include "core/project.h"
#include "renderer/renderer.h"
#include "utils/filesystem.h"

struct AppContext {
  FileSystem filesystem;
  AssetManager assets{filesystem};
  Renderer renderer;
  Project project{filesystem, assets};
  

  // Non-copyable and non-movable: members hold references to each other.
  AppContext() = default;
  AppContext(const AppContext&) = delete;
  AppContext& operator=(const AppContext&) = delete;
  AppContext(AppContext&&) = delete;
  AppContext& operator=(AppContext&&) = delete;
};