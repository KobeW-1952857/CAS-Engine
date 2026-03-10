#pragma once

#include <variant>

#include "core/uuid.h"
#include "scene/entity.h"

using SelectionContext = std::variant<std::monostate, Entity, UUID>;