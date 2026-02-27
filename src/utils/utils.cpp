#include "utils/utils.h"

#include <glad/gl.h>

#include <fstream>
#include <iostream>

#include "Nexus/Log.h"

std::string readFile(const char* filePath) {
  std::string content;
  std::ifstream filestream(filePath, std::ios::in);

  if (!filestream.is_open()) {
    std::cout << "Could not read file " << filePath << ". File does not exist." << std::endl;
    return "";
  }

  std::string line = "";
  while (!filestream.eof()) {
    std::getline(filestream, line);
    content.append(line + "\n");
  }

  filestream.close();
  return content;
}

void checkOpenGLError(const std::string& label) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    Nexus::Logger::error("OpenGL Error [{}]: {}", label, err);
  }
}