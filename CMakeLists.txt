cmake_minimum_required(VERSION 3.20.0)
project(STR VERSION 1.0)
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile_commands.json for IDEs" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER clang++)

include_directories(
    .
    /opt/homebrew/include
    /usr/local/include
)

link_directories(
    /opt/homebrew/lib
    /usr/local/lib
)

set(SOURCES
    src/include/linalg.hpp
    src/camera.cpp
    src/engine.cpp
    src/graphics.cpp
    src/main.cpp
    src/material.cpp
    src/renderer.cpp
    src/transform.cpp
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(STR ${SOURCES})

target_link_libraries(STR
    Vulkan::Vulkan
    glfw
    libvecs.a
)