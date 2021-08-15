# lodge-window

add_library(lodge-window STATIC "")

target_sources(lodge-window
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_window_glfw.c"
		"${CMAKE_CURRENT_LIST_DIR}/util_graphics.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_window.h"
		"${CMAKE_CURRENT_LIST_DIR}/util_graphics.h"
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
