project(glpong)

cmake_minimum_required(VERSION 2.6)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# Build GLFW.
add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)

# Build glew.
add_subdirectory(lib/glew)
include_directories(lib/glew/include)
add_definitions(-DGLEW_STATIC)

# Main executable.
include_directories(${PROJECT_SOURCE_DIR})
add_executable(glpong main.c math4.c)

# Link.
target_link_libraries(glpong libglew_static ${GL_LIBRARY} ${PLATFORM_SPECIFIC_LIBRARIES} glfw ${GLFW_LIBRARIES})
