project(glpong)

cmake_minimum_required(VERSION 2.6)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# Main executable.
include_directories(${PROJECT_SOURCE_DIR})
add_executable(glpong main.c math4.c)

# Build GLFW.
add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)

# Build glew.
add_subdirectory(lib/glew/build/cmake)
include_directories(lib/glew/include)

# Link.
target_link_libraries(glpong glew glfw ${GLFW_LIBRARIES})
