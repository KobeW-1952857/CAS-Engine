#pragma once

#include <Nexus/Window/GLFWWindow.h>

#include <glm/glm.hpp>
#include <vector>

#include "core/asset.h"

struct UniformInfo {
  std::string name;
  GLint size;
  GLenum type;
};

class Shader : public Asset {
 public:
  GLuint m_id;

  Shader() { type = AssetType::Shader; };
  Shader(const char* vertexFilePath, const char* fragmentFilePath);
  Shader& operator=(Shader other);
  ~Shader();

  void use();
  bool compile();
  bool addVertexShader(const char* vertexData);
  bool addFragmentShader(const char* fragmentData);
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
  bool m_hasVertShader;
  bool m_hasFragShader;
  bool m_isLinked;
  bool m_toDeleteProgram;
  GLuint m_vertShaderId;
  GLuint m_fragShaderId;
  bool m_isDirectionalLightSet;

  std::string m_vertexFilePath;
  std::string m_fragmentFilePath;

  bool checkShaderCompileError(unsigned int shader, const char* type);
  bool checkProgramLinkError();
};