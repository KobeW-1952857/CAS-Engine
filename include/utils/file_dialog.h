#pragma once

#include <string>
class FileDialog {
 public:
  static std::string openFolder();
  static std::string openFile(const char* filter = "");
  static std::string saveFile(const char* filter = "");
};