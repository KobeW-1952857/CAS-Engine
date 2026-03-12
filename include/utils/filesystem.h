#pragma once

#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

class FileSystem {
 public:
  FileSystem() {
    m_exe_dir = getExecutableDir();
    m_engine_root = m_exe_dir / ".." / "Resources" / "assets";
  }

  void setProjectRoot(const std::filesystem::path& path) { m_project_root = path; }

  [[nodiscard]] std::filesystem::path resolvePath(const std::filesystem::path& virtual_path) {
    std::string path_str = virtual_path.string();
    if (path_str.starts_with("engine://")) return m_engine_root / path_str.substr(9);
    if (path_str.starts_with("project://")) return m_project_root / path_str.substr(10);
    return virtual_path;
  }

  [[nodiscard]] std::filesystem::path getProjectPath(const std::filesystem::path& absolute_path) {
    std::string path_str = absolute_path.string();
    if (path_str.starts_with("project://")) return absolute_path;
    auto relative_path = absolute_path.lexically_relative(m_project_root);
    return "project://" / relative_path;
  }

  [[nodiscard]] const std::filesystem::path& getProjectRoot() const { return m_project_root; }
  [[nodiscard]] const std::filesystem::path& getEngineRoot() const { return m_engine_root; }

  [[nodiscard]] std::filesystem::path getExecutableDir() {
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

  std::filesystem::path m_exe_dir = FileSystem::getExecutableDir();
  std::filesystem::path m_engine_root;
  std::filesystem::path m_project_root;
};