# lodge-gui

add_library(lodge-gui STATIC "")

target_sources(lodge-gui
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_gui.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_gui.h"
)

target_include_directories(lodge-gui
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/nuklear"
)

target_link_libraries(lodge-gui
	PRIVATE
		lodge-build-flags
		lodge-gfx
	PUBLIC
		lodge-lib
)
