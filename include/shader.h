#pragma once

#include <Nexus/Window/GLFWWindow.h>
#include <glm/glm.hpp>

class Shader {
public:
  GLuint m_id;

  Shader(const char *vertexFilePath, const char *fragmentFilePath);
  Shader &operator=(Shader other);
  ~Shader();

  void use();
  bool compile();
  bool addVertexShader(const char *vertexData);
  bool addFragmentShader(const char *fragmentData);
  bool linkProgram();

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setMat4(const std::string &name, glm::mat4 value) const;
  void setVec3(const std::string &name, glm::vec3 value) const;
  void setVec4(const std::string &name, glm::vec4 value) const;
  void setMVP(const glm::mat4 &model, const glm::mat4 &view,
              const glm::mat4 &projection) const;

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


  bool checkShaderCompileError(unsigned int shader, const char *type);
  bool checkProgramLinkError();
};