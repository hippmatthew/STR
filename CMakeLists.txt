cmake_minimum_required(VERSION 3.20.0)
project(str VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER clang++)

find_program(GLSLC glslc REQUIRED)

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
    ${CMAKE_SOURCE_DIR}/src/include/linalg.hpp
    ${CMAKE_SOURCE_DIR}/src/camera.cpp
    ${CMAKE_SOURCE_DIR}/src/engine.cpp
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/transform.cpp
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(str ${SOURCES})

target_link_libraries(str
    Vulkan::Vulkan
    glfw
    libvecs.a
)

set(SHADERS
  ${CMAKE_SOURCE_DIR}/shaders/camera.vert
  ${CMAKE_SOURCE_DIR}/shaders/camera.frag
)

set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)

foreach(SHADER ${SHADERS})
  get_filename_component(FILE_NAME ${SHADER} NAME)
  get_filename_component(FILE_EXT ${SHADER} EXT)
  set(SPV ${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv)

  add_custom_command(
    OUTPUT ${SPV}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
    COMMAND ${GLSLC} -o ${SPV} ${SHADER}
    DEPENDS ${SHADER}
    COMMENT "Compiling ${SHADER}"
  )

  list(APPEND SPVS ${SPV})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPVS})
add_dependencies(str shaders)