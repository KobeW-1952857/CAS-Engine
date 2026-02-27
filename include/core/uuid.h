#pragma once

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
  std::size_t operator()(const UUID& uuid) const { return std::hash<uint64_t>{}(uuid); }
};

}  // namespace std