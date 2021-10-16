# Library: GLFW.
set(BUILD_SHARED_LIBS ${ENABLE_SHARED} CACHE BOOL "Build shared libraries" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
set(GLFW_USE_RETINA OFF CACHE BOOL "Disable GLFW retina scaling" FORCE)
add_subdirectory(lib/glfw ${CMAKE_BINARY_DIR}/lib/glfw)

# lodge-window

add_library(lodge-window STATIC "")

target_sources(lodge-window
	PRIVATE
		"lodge_window_glfw.c"
	PUBLIC
		"lodge_window.h"
)

target_include_directories(lodge-window
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-window
	PRIVATE
		lodge-build-flags
		${GLEW_LIBRARY}
		glfw
	PUBLIC
		lodge-lib
		lodge-plugins		# FIXME(TS): should be separate module
		lodge-gfx			# FIXME(TS): not sure about this dependency
)
