project(glpong)

cmake_minimum_required(VERSION 3.1)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# QUIRK: Define M_PI on Windows.
add_definitions(-D_USE_MATH_DEFINES)

# Add debug statements.
add_definitions(-DDEBUG)

# GCC: More compiler warnings.
if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall)
endif()

if(NOT EMSCRIPTEN)
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
else()
    # glfw and glew is provided by emscripten
    option(USE_GLFW_3 "Use glfw3" ON)
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths")
endif()

# Include stb.h
include_directories(lib/)

# Main executable.
add_executable(glpong main.c math4.c graphics.c input.c texture.c color.c)

# Link.
if(NOT EMSCRIPTEN)
    # Build glpong using the C99 standard.
    set_property(TARGET glpong PROPERTY C_STANDARD 99)
    target_link_libraries(glpong libglew_static ${GL_LIBRARY} ${PLATFORM_SPECIFIC_LIBRARIES} glfw ${GLFW_LIBRARIES})
else()
    target_link_libraries(glpong)
endif()
