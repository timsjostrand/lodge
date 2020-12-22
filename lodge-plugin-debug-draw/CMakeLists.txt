add_library(lodge-plugin-debug-draw STATIC "")

target_sources(lodge-plugin-debug-draw
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_debug_draw.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_debug_draw.h"
)

target_include_directories(lodge-plugin-debug-draw
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-debug-draw
	PRIVATE
		lodge-build-flags
        lodge-plugin-shaders
	PUBLIC
		lodge-plugins
		lodge-entity
		lodge-plugin-scene-renderer
)
