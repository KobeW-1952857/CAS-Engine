#pragma once

// IWYU pragma: begin_exports
#include <imgui.h>
#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/app_context.h"
#include "scene/entity.h"
#include "utils/yaml_extension.h"
// IWYU pragma: end_exports

struct ComponentDefaults {
  static constexpr bool user_addable = true;
  static constexpr bool user_removable = true;
  static constexpr bool user_editable = true;
};