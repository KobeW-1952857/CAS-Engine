cmake_minimum_required(VERSION 3.20)

project(CASEngine CXX)

find_package(Nexus REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)


set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/CASEngine.icon")
set_source_files_properties(${APP_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

add_executable(CASEngine
    src/core/asset_manager.cpp
    src/core/input.cpp
    src/core/project.cpp
    src/core/uuid.cpp
    src/core/simulation_clock.cpp
    src/editor/editor.cpp
    src/editor/editor_camera.cpp
    src/editor/grid_overlay.cpp
    src/editor/panels/asset_browser.cpp
    src/editor/panels/console.cpp
    src/editor/panels/properties.cpp
    src/editor/panels/scene_hierarchy.cpp
    src/renderer/camera.cpp
    src/renderer/framebuffer.cpp
    src/renderer/material.cpp
    src/renderer/primitives/mesh.cpp
    src/renderer/renderer.cpp
    src/renderer/shader.cpp
    src/renderer/systems/mesh_renderer_system.cpp
    src/renderer/systems/line_renderer_system.cpp
    src/renderer/systems/bezier_renderer_system.cpp
    src/renderer/tesselation_shader.cpp
    src/renderer/texture.cpp
    src/scene/scene.cpp
    src/scene/systems/line_follower_system.cpp
    src/utils/file_dialog.mm
    src/utils/utils.cpp

    # TEMP: move it to it's own cmake file/library or something
    vendor/ImGuizmo/ImGuizmo.cpp

    src/main.cpp

    ${APP_ICON}
)

set_target_properties(CASEngine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

target_compile_definitions(CASEngine PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(CASEngine PRIVATE Nexus::Nexus)
target_link_libraries(CASEngine PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(CASEngine PUBLIC "-framework Cocoa" "-framework UniformTypeIdentifiers")
target_include_directories(CASEngine PUBLIC include vendor/stb/include vendor/tiny_obj/include vendor/EnTT/include vendor/IconsFontAwesome vendor/ImGuizmo)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf 
        ${CMAKE_BINARY_DIR}/CMakeCache.txt
        ${CMAKE_BINARY_DIR}/CMakeFiles
        ${CMAKE_BINARY_DIR}/CASEngine
    COMMENT "Removing build artifacts..."
)


file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "assets/*")

if (APPLE)
    set(ASSET_DEST_DIR "$<TARGET_BUNDLE_CONTENT_DIR:CASEngine>/Resources/assets")
    add_custom_command(TARGET CASEngine POST_BUILD
        COMMAND codesign --entitlements "${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist" -f -s "-" $<TARGET_FILE:CASEngine>
        COMMENT "Codesigning CASEngine with file access entitlements..."
    )
else()
    set(ASSET_DEST_DIR "$<TARGET_FILE_DIR:CASEngine>/assets")
endif()
set(ASSET_STAMP "${CMAKE_CURRENT_BINARY_DIR}/assets.stamp")

add_custom_command(
    OUTPUT ${ASSET_STAMP}
    DEPENDS ${ASSET_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${ASSET_DEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${ASSET_STAMP}
    COMMENT "Assets changed! Syncing to build directory..."
)

add_custom_target(SyncAssets ALL DEPENDS ${ASSET_STAMP})
add_dependencies(CASEngine SyncAssets)