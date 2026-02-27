cmake_minimum_required(VERSION 3.20)

project(Planets CXX)

find_package(Nexus REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)

add_executable(Planets
	src/core/asset_manager.cpp
    src/core/input.cpp
    src/core/resource_manager.cpp
    src/core/uuid.cpp
    src/editor/scene_hierarchy.cpp
    src/editor/editor_camera.cpp
    src/editor/editor.cpp
    src/main.cpp
    src/renderer/camera.cpp
    src/renderer/framebuffer.cpp
    src/renderer/material.cpp
    src/renderer/mesh.cpp
    src/renderer/shader.cpp
    src/renderer/tesselation_shader.cpp
    src/renderer/texture.cpp
    src/scene/scene.cpp
    src/scene/scene_serializer.cpp
    # src/tmp/bezier_curve.cpp
    # src/tmp/planet.cpp
    src/utils/file_dialog.mm
    src/utils/utils.cpp
)

set_target_properties(Planets PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(Planets PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(Planets PRIVATE Nexus::Nexus)
target_link_libraries(Planets PRIVATE yaml-cpp::yaml-cpp)
target_include_directories(Planets PUBLIC include vendor/stb/include vendor/tiny_obj/include vendor/EnTT/include)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf 
        ${CMAKE_BINARY_DIR}/CMakeCache.txt
        ${CMAKE_BINARY_DIR}/CMakeFiles
        ${CMAKE_BINARY_DIR}/Planets
    COMMENT "Removing build artifacts..."
)