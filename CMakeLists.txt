project(glpong)

cmake_minimum_required(VERSION 2.6)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

add_definitions(-D_USE_MATH_DEFINES) 

# Build GLFW.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)

# Build glew.
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "Build the shared glew library")
add_subdirectory(lib/glew)
include_directories(lib/glew/include)
add_definitions(-DGLEW_STATIC)

# Main executable.
include_directories(${PROJECT_SOURCE_DIR})
add_executable(glpong main.c math4.c)

# Link.
target_link_libraries(glpong libglew_static ${GL_LIBRARY} ${PLATFORM_SPECIFIC_LIBRARIES} glfw ${GLFW_LIBRARIES})
