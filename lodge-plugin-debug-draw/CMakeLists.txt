add_library(lodge-plugin-debug-draw STATIC "")

target_sources(lodge-plugin-debug-draw
	PRIVATE
		"lodge_plugin_debug_draw.c"
	PUBLIC
		"lodge_plugin_debug_draw.h"
)

target_include_directories(lodge-plugin-debug-draw
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-debug-draw
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-plugins
		lodge-entity
		lodge-plugin-shaders
		lodge-plugin-scene-renderer
)

lodge_target_make_plugin(lodge-plugin-debug-draw "lodge_plugin_debug_draw.h" lodge_plugin_debug_draw)