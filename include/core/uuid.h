#pragma once

#include <yaml-cpp/yaml.h>

#include <cstddef>
#include <cstdint>
#include <functional>

class UUID {
 public:
  UUID();
  UUID(uint64_t uuid) : m_uuid(uuid) {}

  operator uint64_t() const { return m_uuid; }
  explicit operator bool() const { return m_uuid != 0; }
  bool operator==(const UUID& other) const { return m_uuid == other.m_uuid; }
  bool operator!=(const UUID& other) const { return m_uuid != other.m_uuid; }

 private:
  uint64_t m_uuid;
};

namespace std {
template <>
struct hash<UUID> {
  std::size_t operator()(const UUID& uuid) const { return std::hash<uint64_t>{}(static_cast<uint64_t>(uuid)); }
};

}  // namespace std

namespace YAML {
template <>
struct convert<UUID> {
  static Node encode(const UUID& uuid) {
    Node node;
    node = static_cast<uint64_t>(uuid);
    return node;
  }

  static bool decode(const Node& node, UUID& uuid) {
    if (!node.IsScalar()) return false;
    uuid = UUID(node.as<uint64_t>());
    return true;
  }
};
}  // namespace YAML