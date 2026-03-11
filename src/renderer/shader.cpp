#include "renderer/shader.h"

#include <Nexus/Log.h>
#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "utils/filesystem.h"
#include "utils/utils.h"

bool Shader::compile() { return linkProgram(); }

static const std::unordered_map<std::string, ShaderStage> extension_to_stage = {
    {".vert", ShaderStage::Vertex},      {".frag", ShaderStage::Fragment},       {".geom", ShaderStage::Geometry},
    {".tesc", ShaderStage::TessControl}, {".tese", ShaderStage::TessEvaluation},
};

Shader::Shader(const std::filesystem::path& path) {
  type = AssetType::Shader;
  m_id = glCreateProgram();

  for (const auto& [ext, stage] : extension_to_stage) {
    auto stage_path = path.parent_path() / (path.stem().string() + ext);
    if (std::filesystem::exists(stage_path)) {
      addStage(stage, stage_path);
    }
  }
  linkProgram();
}

Shader::~Shader() { glDeleteProgram(m_id); }

static std::string preprocessIncludes(const std::string& source, const std::filesystem::path& shader_dir,
                                      std::unordered_set<std::string>& visited) {
  std::string result;
  std::istringstream stream(source);
  std::string line;

  while (std::getline(stream, line)) {
    auto hash = line.find("#include");
    if (hash != std::string::npos) {
      auto start = line.find('"', hash);
      auto end = line.rfind('"');
      if (start != std::string::npos && start != end) {
        std::string inc = line.substr(start + 1, end - start - 1);

        std::filesystem::path resolved;
        if (inc.starts_with("engine://") || inc.starts_with("project://")) {
          resolved = FileSystem::resolvePath(inc);
        } else {
          auto rel = shader_dir / inc;
          resolved = std::filesystem::exists(rel) ? rel : FileSystem::resolvePath("engine://shaders/" + inc);
        }

        auto key = resolved.string();
        if (!visited.contains(key) && std::filesystem::exists(resolved)) {
          visited.insert(key);
          result += preprocessIncludes(readFile(resolved.c_str()), resolved.parent_path(), visited);
        } else if (!std::filesystem::exists(resolved)) {
          Nexus::Logger::error("Shader include not found: {}", inc);
        }
        continue;
      }
    }

    result += line + "\n";
  }
  return result;
}

bool Shader::addStage(ShaderStage stage, const std::filesystem::path& path) {
  m_stagePaths[stage] = path;
  if (!m_isLinked) {
    auto source = readFile(path.c_str());
    auto visited = std::unordered_set<std::string>{path.string()};
    source = preprocessIncludes(source, path.parent_path(), visited);

    return addShaderStage(stage, source.c_str());
  }

  Nexus::Logger::debug("Shader stage [{}] queued - call relink() to apply", stageName(stage));
  return true;
}

void Shader::removeStage(ShaderStage stage) {
  if (!hasStage(stage)) {
    Nexus::Logger::warn("Shader::removeStage - stage [{}] not present", stageName(stage));
    return;
  }
  m_stagePaths.erase(stage);
  Nexus::Logger::debug("Shader stage [{}] removed - call relink() to apply", stageName(stage));
}

bool Shader::hasStage(ShaderStage stage) { return m_stagePaths.find(stage) != m_stagePaths.end(); }

bool Shader::relink() {
  if (!m_isLinked) return linkProgram();
  m_isLinked = false;

  GLuint old_program = m_id;
  auto old_stage_paths = m_stagePaths;
  m_stagePaths.clear();

  bool success = false;
  m_id = glCreateProgram();

  auto reset = [&]() {
    for (auto& [stage, id] : m_stageIds) glDeleteShader(id);
    m_stageIds.clear();
    m_stagePaths = old_stage_paths;
    m_isLinked = true;
    glDeleteProgram(m_id);
    m_id = old_program;
    return false;
  };

  for (const auto& [stage, path] : old_stage_paths) success |= addStage(stage, path);
  if (!success) return reset();
  if (!linkProgram()) return reset();

  Nexus::Logger::info("Shader relinked successfully ({} stages)", m_stagePaths.size());
  return true;
}

std::vector<UniformInfo> Shader::getActiveUniforms() const {
  std::vector<UniformInfo> uniforms;
  GLint numUniforms;
  glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &numUniforms);

  const GLsizei buf_size = 256;
  GLchar name_buffer[buf_size];

  for (GLint i = 0; i < numUniforms; ++i) {
    // Skip uniforms that belong to a UBO block
    GLint block_index;
    glGetActiveUniformsiv(m_id, 1, reinterpret_cast<GLuint*>(&i), GL_UNIFORM_BLOCK_INDEX, &block_index);
    if (block_index != -1) continue;

    UniformInfo uniform{};
    GLsizei length;
    glGetActiveUniform(m_id, static_cast<GLuint>(i), buf_size, &length, &uniform.size, &uniform.type, name_buffer);
    uniform.name = std::string(name_buffer, length);

    uniforms.push_back(uniform);
  }
  return uniforms;
}

void Shader::use() {
  if (!m_isLinked) linkProgram();
  glUseProgram(m_id);
}

void Shader::setBool(const std::string& name, bool value) const {
  glUniform1i(getUniformLocation(name), static_cast<int>(value));
}
void Shader::setInt(const std::string& name, int value) const { glUniform1i(getUniformLocation(name), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(getUniformLocation(name), value); }
void Shader::setMat4(const std::string& name, glm::mat4 value) const {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec2(const std::string& name, glm::vec2 value) const {
  glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void Shader::setVec3(const std::string& name, glm::vec3 value) const {
  glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void Shader::setVec4(const std::string& name, glm::vec4 value) const {
  glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const {
  setMat4("model", model);
  setMat4("view", view);
  setMat4("projection", projection);
}

// Private functions

GLenum Shader::toGLenum(ShaderStage stage) {
  switch (stage) {
    case ShaderStage::Vertex:
      return GL_VERTEX_SHADER;
    case ShaderStage::Fragment:
      return GL_FRAGMENT_SHADER;
    case ShaderStage::TessControl:
      return GL_TESS_CONTROL_SHADER;
    case ShaderStage::TessEvaluation:
      return GL_TESS_EVALUATION_SHADER;
    case ShaderStage::Geometry:
      return GL_GEOMETRY_SHADER;
      // case ShaderStage::Compute:
      //   return GL_COMPUTE_SHADER;
  }
}

const char* Shader::stageName(ShaderStage stage) {
  switch (stage) {
    case ShaderStage::Vertex:
      return "Vertex";
    case ShaderStage::Fragment:
      return "Fragment";
    case ShaderStage::TessControl:
      return "TessControl";
    case ShaderStage::TessEvaluation:
      return "TessEvaluation";
    case ShaderStage::Geometry:
      return "Geometry";
      // case ShaderStage::Compute:
      //   return "Compute";
  }
}

bool Shader::checkShaderCompileError(GLuint shader, ShaderStage stage) {
  int success;
  char infoLog[1024];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
    Nexus::Logger::error("Shader compile error [{}]: {}", stageName(stage), infoLog);
  }
  return success;
}
bool Shader::checkProgramLinkError() {
  int success;
  char infoLog[1024];
  glGetProgramiv(m_id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(m_id, 1024, nullptr, infoLog);
    Nexus::Logger::error("Shader program link error: {}", infoLog);
  }
  return success;
}

bool Shader::addShaderStage(ShaderStage stage, const char* sourceData) {
  GLuint shader_id = glCreateShader(toGLenum(stage));
  glShaderSource(shader_id, 1, &sourceData, nullptr);
  glCompileShader(shader_id);

  if (!checkShaderCompileError(shader_id, stage)) {
    glDeleteShader(shader_id);
    return false;
  }

  glAttachShader(m_id, shader_id);
  m_stageIds[stage] = shader_id;
  return true;
}

bool Shader::linkProgram() {
  bool hasBasicStages = hasStage(ShaderStage::Vertex) && hasStage(ShaderStage::Fragment);

  if (!hasBasicStages) {
    Nexus::Logger::error("Shader link failed: missing Vertex or Fragment stage");
    return false;
  }

  glLinkProgram(m_id);
  if (!checkProgramLinkError()) return false;

  for (auto& [stage, id] : m_stageIds) glDeleteShader(id);

  GLuint idx = glGetUniformBlockIndex(m_id, "PerFrame");
  if (idx != GL_INVALID_INDEX) glUniformBlockBinding(m_id, idx, 0);
  idx = glGetUniformBlockIndex(m_id, "PerDraw");
  if (idx != GL_INVALID_INDEX) glUniformBlockBinding(m_id, idx, 1);

  m_stageIds.clear();
  m_uniformLocationCache.clear();
  m_isLinked = true;
  return true;
}

GLint Shader::getUniformLocation(const std::string& name) const {
  if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
    return m_uniformLocationCache[name];
  }

  GLint location = glGetUniformLocation(m_id, name.c_str());
  m_uniformLocationCache[name] = location;
  return location;
}