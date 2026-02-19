cmake_minimum_required(VERSION 3.15)

project(Planets CXX)

find_package(Nexus REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_STANDARD 20)

file(GLOB SRC_FILES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.h" "include/*.hpp")
list(APPEND HEADER_FILES vendor/tiny_obj_loader.h)

add_executable(Planets)

target_link_libraries(Planets PRIVATE Nexus::Nexus)
target_sources("Planets" PRIVATE ${SRC_FILES})
target_include_directories("Planets" PUBLIC include)