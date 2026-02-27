#pragma once

#include "core/uuid.h"
enum class AssetType { None = 0, Texture, Mesh, Shader, Material };

class Asset {
 public:
  UUID handle;
  AssetType type;
  virtual ~Asset() = default;
};