#include "renderer/mesh.h"

#include "Nexus/Log.h"
#include "Nexus/Window/GLFWWindow.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::Mesh(std::string filepath) : m_filepath(filepath) {
  type = AssetType::Mesh;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());
  if (!ret) {
    Nexus::Logger::error("Could not open file: {}", filepath);
    return;
  }

  for (const auto& shape : shapes) {
    m_indices.reserve(shape.mesh.indices.size());  // Reserve space for indices

    m_name = shape.name;
    m_vertices.reserve(shape.mesh.indices.size());

    for (const auto& index : shape.mesh.indices) {
      Vertex vertex{};  // Initialize all members to default values

      // Position
      vertex.position = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
                         attrib.vertices[3 * index.vertex_index + 2]};

      // Normal
      if (!attrib.normals.empty() && index.normal_index != -1) {
        vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
                         attrib.normals[3 * index.normal_index + 2]};
      } else {
        // Default normal if not present in OBJ
        vertex.normal = {0.0f, 0.0f, 0.0f};
      }

      // Texture Coordinates
      if (!attrib.texcoords.empty() && index.texcoord_index != -1) {
        vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                           attrib.texcoords[2 * index.texcoord_index + 1]};
      } else {
        // Default texture coordinates if not present in OBJ
        vertex.texCoord = {0.0f, 0.0f};
      }

      // Color (assuming per-vertex color, indexed by vertex_index)
      // tinyobjloader typically loads 3-component colors. We default alpha to 1.0.
      if (!attrib.colors.empty() && (3 * index.vertex_index + 2) < attrib.colors.size()) {
        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0], attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
            1.0f  // Default alpha to 1.0
        };
      } else {
        // Default color to white if not present in OBJ
        vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};
      }
      m_indices.push_back(static_cast<GLuint>(m_vertices.size()));  // Index into the m_vertices array
      m_vertices.push_back(vertex);
    }
  }

  initOpenGLBuffers();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
    : m_vertices(vertices), m_indices(indices) {
  type = AssetType::Mesh;
  initOpenGLBuffers();
}

void Mesh::render() {
  if (!m_initialized) return;

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Mesh::initOpenGLBuffers() {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
  glBindVertexArray(0);

  m_initialized = true;
}