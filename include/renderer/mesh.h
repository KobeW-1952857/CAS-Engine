#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Nexus/Window/GLFWWindow.h"
#include "core/asset.h"
#include "core/asset_traits.h"
#include "utils/utils.h"

struct Vertex {
  Point position;
  Direction normal;
  TextureCoord texCoord;
  Color color;
  Direction smoothed_normal;
};

class Mesh : public Asset {
 public:
  Mesh() { type = AssetType::Mesh; }
  Mesh(std::string filepath);
  Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

  void render();

  const std::string& getName() const { return m_name; }
  const std::string& getFilePath() const { return m_filepath; }

  operator bool() const { return m_initialized; }

 private:
  void initOpenGLBuffers();

 private:
  std::string m_name;
  std::string m_filepath;
  bool m_initialized = false;
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  GLuint m_vao, m_vbo, m_ebo;

  void computeSmoothedNormals();
};

template <>
struct AssetTraits<Mesh> {
  static constexpr AssetType type = AssetType::Mesh;
  static constexpr const char* default_name = "New Mesh";
  static constexpr const char* extension = ".obj";

  static bool matchesExtension(std::string_view ext) { return ext == extension; }

  static std::shared_ptr<Mesh> load(const std::filesystem::path& path, AssetManager& assets) {
    return std::make_shared<Mesh>(path.string());
  }
  static void save(const Mesh& asset, const std::filesystem::path& path, AssetManager& assets) {}

  static void initializeNew(Mesh& asset, AssetManager& assets) {}
};