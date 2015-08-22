project(glpong)

cmake_minimum_required(VERSION 3.1)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# QUIRK: Define M_PI on Windows.
add_definitions(-D_USE_MATH_DEFINES)

if( NOT EMSCRIPTEN ) # glfw and glew is provided by emscripten
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
	option(USE_GLFW_3 "Use glfw3" ON)
    #set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -lglfw3)
	#set(CMAKE_EXE_LINKER_FLAGS -lglfw)
endif()

# Main executable.
include_directories(${PROJECT_SOURCE_DIR})
add_executable(glpong main.c math4.c)

# Build glpong using the C99 standard.
set_property(TARGET glpong PROPERTY C_STANDARD 99)

# Link.
if( NOT EMSCRIPTEN )
	target_link_libraries(glpong libglew_static ${GL_LIBRARY} ${PLATFORM_SPECIFIC_LIBRARIES} glfw ${GLFW_LIBRARIES})
else()
	target_link_libraries(glpong libglfw3.a)
endif()
