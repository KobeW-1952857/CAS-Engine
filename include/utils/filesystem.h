#pragma once

#include <filesystem>

#include "Nexus/Log.h"
#include "core/project.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

class FileSystem {
 public:
  static std::filesystem::path resolvePath(const std::filesystem::path& virtual_path) {
    std::string path_str = virtual_path.string();
    if (path_str.starts_with("engine://")) return s_engine_root / path_str.substr(9);
    if (path_str.starts_with("project://")) return s_project_root / path_str.substr(10);
    return virtual_path;
  }

  static std::filesystem::path getProjectPath(const std::filesystem::path& absolute_path) {
    std::string path_str = absolute_path.string();
    if (path_str.starts_with("project://")) return absolute_path;
    auto relative_path = absolute_path.lexically_relative(s_project_root);
    return "project://" / relative_path;
  }

  static std::filesystem::path getExecutableDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    // Returns the folder containing the .exe
    return std::filesystem::path(path).parent_path();
#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
      return std::filesystem::path(path).parent_path();
    }
    return std::filesystem::current_path();
#else
    return std::filesystem::current_path();
#endif
  }

  inline static std::filesystem::path s_exe_dir = FileSystem::getExecutableDir();
  inline static std::filesystem::path s_engine_root = s_exe_dir / ".." / "Resources" / "assets";
  inline static std::filesystem::path s_project_root = Project::getConfig().path;
};