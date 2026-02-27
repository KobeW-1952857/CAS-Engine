#include "renderer/tesselation_shader.h"

#include <iostream>

#include "utils/utils.h"

TesselationShader::TesselationShader(std::string name, const char* vShaderFile, const char* fShaderFile,
                                     const char* tcShaderFile, const char* teShaderFile) {
  m_id = glCreateProgram();

  addVertexShader(readFile(vShaderFile).c_str());
  addFragmentShader(readFile(fShaderFile).c_str());
  addTesselationControlShader(readFile(tcShaderFile).c_str());
  addTesselationEvaluationShader(readFile(teShaderFile).c_str());

  linkProgram();
}

TesselationShader::~TesselationShader() { glDeleteProgram(m_id); }

void TesselationShader::addVertexShader(const char* vertexData) {
  m_vShaderId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(m_vShaderId, 1, &vertexData, nullptr);
  glCompileShader(m_vShaderId);

  int success;
  char infoLog[512];
  glGetShaderiv(m_vShaderId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(m_vShaderId, 512, nullptr, infoLog);
    throw std::runtime_error(infoLog);
  };
  m_hasVertShader = true;
}

void TesselationShader::addFragmentShader(const char* fragmentData) {
  m_fShaderId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(m_fShaderId, 1, &fragmentData, nullptr);
  glCompileShader(m_fShaderId);
  int success;
  char infoLog[512];
  glGetShaderiv(m_fShaderId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(m_fShaderId, 512, nullptr, infoLog);
    throw std::runtime_error(infoLog);
  };
  m_hasFragShader = true;
}
void TesselationShader::addTesselationControlShader(const char* tcData) {
  m_tcShaderId = glCreateShader(GL_TESS_CONTROL_SHADER);
  glShaderSource(m_tcShaderId, 1, &tcData, nullptr);
  glCompileShader(m_tcShaderId);
  int success;
  char infoLog[512];
  glGetShaderiv(m_tcShaderId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(m_tcShaderId, 512, nullptr, infoLog);
    throw std::runtime_error(infoLog);
  };
  m_hasTesselationControlShader = true;
}
void TesselationShader::addTesselationEvaluationShader(const char* teData) {
  m_teShaderId = glCreateShader(GL_TESS_EVALUATION_SHADER);
  glShaderSource(m_teShaderId, 1, &teData, nullptr);
  glCompileShader(m_teShaderId);
  int success;
  char infoLog[512];
  glGetShaderiv(m_teShaderId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(m_teShaderId, 512, nullptr, infoLog);
    throw std::runtime_error(infoLog);
  };
  m_hasTesselationEvaluationShader = true;
}

void TesselationShader::linkProgram() {
  if (!m_hasVertShader || !m_hasFragShader || !m_hasTesselationControlShader || !m_hasTesselationEvaluationShader) {
    std::cout << "ERROR::PROGRAM::LINKING_FAILED::NO_SHADERS_ADDED" << std::endl;
    return;
  }
  glAttachShader(m_id, m_vShaderId);
  glAttachShader(m_id, m_fShaderId);
  glAttachShader(m_id, m_tcShaderId);
  glAttachShader(m_id, m_teShaderId);
  glLinkProgram(m_id);

  int success;
  char infoLog[512];
  glGetProgramiv(m_id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(m_id, 512, nullptr, infoLog);
    std::cout << "ERROR::PROGRAM::" << m_id << "::LINKING_FAILED" << infoLog << std::endl;
  }
}