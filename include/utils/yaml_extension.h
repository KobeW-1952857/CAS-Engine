#pragma once

#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>

namespace YAML {
template <int L, typename T, glm::qualifier Q>
struct convert<glm::vec<L, T, Q>> {
  static Node encode(const glm::vec<L, T, Q>& vec) {
    Node node;
    for (int i = 0; i < L; ++i) {
      node.push_back(vec[i]);
    }
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, glm::vec<L, T, Q>& vec) {
    if (!node.IsSequence() || node.size() != L) return false;
    for (int i = 0; i < L; ++i) {
      vec[i] = node[i].as<T>();
    }
    return true;
  }
};
template <int L, typename T, glm::qualifier Q>
Emitter& operator<<(Emitter& out, const glm::vec<L, T, Q>& v) {
  out << Flow << BeginSeq;
  for (int i = 0; i < L; ++i) {
    out << v[i];
  }
  out << EndSeq;
  return out;
}

template <int C, int R, typename T, glm::qualifier Q>
struct convert<glm::mat<C, R, T, Q>> {
  static Node encode(const glm::mat<C, R, T, Q>& mat) {
    Node node;
    // GLM matrices are accessed via mat[column][row]
    for (int c = 0; c < C; ++c) {
      for (int r = 0; r < R; ++r) {
        node.push_back(mat[c][r]);
      }
    }
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, glm::mat<C, R, T, Q>& mat) {
    // A matrix must be a sequence of exactly C * R elements
    if (!node.IsSequence() || node.size() != C * R) return false;

    for (int c = 0; c < C; ++c) {
      for (int r = 0; r < R; ++r) {
        // Calculate the flat 1D index
        mat[c][r] = node[c * R + r].as<T>();
      }
    }
    return true;
  }
};
template <int C, int R, typename T, glm::qualifier Q>
Emitter& operator<<(Emitter& out, const glm::mat<C, R, T, Q>& m) {
  out << Flow << BeginSeq;
  for (int c = 0; c < C; ++c) {
    for (int r = 0; r < R; ++r) {
      out << m[c][r];
    }
  }
  out << EndSeq;
  return out;
}

}  // namespace YAML