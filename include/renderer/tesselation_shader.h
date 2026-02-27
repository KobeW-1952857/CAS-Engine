#pragma once

#include "Nexus/Window/GLFWWindow.h"
class TesselationShader {
 public:
  TesselationShader(std::string name, const char* vShaderFile, const char* fShaderFile, const char* tcShaderFile,
                    const char* teShaderFile);
  ~TesselationShader();

  void addVertexShader(const char* vertexData);
  void addFragmentShader(const char* fragmentData);
  void addTesselationControlShader(const char* controlData);
  void addTesselationEvaluationShader(const char* evaluationData);
  void linkProgram();

  void use() { glUseProgram(m_id); }

 private:
  GLuint m_id;
  GLuint m_vShaderId, m_fShaderId, m_tcShaderId, m_teShaderId;
  bool m_hasVertShader, m_hasFragShader, m_hasTesselationControlShader, m_hasTesselationEvaluationShader;
  bool m_isLinked;
};