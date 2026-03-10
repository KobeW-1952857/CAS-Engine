#pragma once

#include <filesystem>

#include "Nexus/Log.h"
#include "core/uuid.h"
enum class AssetType : uint8_t { None = 0, Texture, Mesh, Shader, Material };

class Asset {
 public:
  UUID handle;
  AssetType type;
  bool modified = false;
  virtual ~Asset() = default;
  virtual void serialize(const std::filesystem::path& filepath) const {
    Nexus::Logger::critical("Serializer for {} not implemented, but called", static_cast<uint8_t>(type));
  }
};

namespace YAML {
template <>
struct convert<AssetType> {
  static Node encode(const AssetType& type) {
    Node node;
    node = static_cast<uint8_t>(type);
    return node;
  }

  static bool decode(const Node& node, AssetType& type) {
    if (!node.IsScalar()) return false;
    type = static_cast<AssetType>(node.as<uint8_t>());
    return true;
  }
};

inline Emitter& operator<<(Emitter& out, const AssetType& type) {
  out << Flow << static_cast<uint8_t>(type);
  return out;
}

}  // namespace YAML
