#pragma once

#include <Nexus/Window/GLFWWindow.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/asset.h"
#include "core/asset_traits.h"

class FileSystem;
class AssetManager;

struct UniformInfo {
  std::string name;
  GLint size;
  GLenum type;
};

enum class ShaderStage : uint8_t { Vertex, Fragment, TessControl, TessEvaluation, Geometry /*, Compute*/ };

class Shader : public Asset {
 public:
  GLuint m_id;

  Shader() {
    m_id = glCreateProgram();
    type = AssetType::Shader;
  };
  Shader(const std::filesystem::path& path, FileSystem& fs);
  Shader& operator=(Shader other);
  ~Shader();

  void use();
  bool compile();
  bool addStage(ShaderStage stage, const std::filesystem::path& path, FileSystem& fs);
  void removeStage(ShaderStage stage);
  bool hasStage(ShaderStage stage);
  bool relink(FileSystem& fs);
  bool linkProgram();

  std::vector<UniformInfo> getActiveUniforms() const;

  void setBool(const std::string& name, bool value) const;
  void setInt(const std::string& name, int value) const;
  void setFloat(const std::string& name, float value) const;
  void setMat4(const std::string& name, glm::mat4 value) const;
  void setVec2(const std::string& name, glm::vec2 value) const;
  void setVec3(const std::string& name, glm::vec3 value) const;
  void setVec4(const std::string& name, glm::vec4 value) const;
  void setMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const;

 private:
  std::unordered_map<ShaderStage, GLuint> m_stageIds;
  std::unordered_map<ShaderStage, std::string> m_stagePaths;
  mutable std::unordered_map<std::string, GLint> m_uniformLocationCache;

  bool m_isLinked = false;

  static GLenum toGLenum(ShaderStage stage);
  static const char* stageName(ShaderStage stage);
  bool addShaderStage(ShaderStage stage, const char* sourceData);
  bool checkShaderCompileError(GLuint shader, ShaderStage type);
  bool checkProgramLinkError();
  GLint getUniformLocation(const std::string& name) const;
};

template <>
struct AssetTraits<Shader> {
  static constexpr AssetType type = AssetType::Shader;
  static constexpr const char* default_name = "New Shader";
  static constexpr const char* extension = ".glsl";

  static bool matchesExtension(std::string_view ext) {
    return ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".geom" || ext == ".tesc" || ext == ".tese";
  }

  static std::shared_ptr<Shader> load(const std::filesystem::path& path, AssetManager& assets);
  static void save(const Shader& asset, const std::filesystem::path& path, AssetManager& assets) {}

  static void initializeNew(Shader& asset, AssetManager& assets) {}
};