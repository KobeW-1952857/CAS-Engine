#pragma once

#include <tuple>

#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "scene/scene.h"
// #include "renderer/texture.h"

using AllAssetTypes = std::tuple<Mesh, Shader, Material, Scene>;