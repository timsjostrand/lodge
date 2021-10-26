# lodge-plugin-terrain

add_library(lodge-plugin-terrain STATIC "")

target_sources(lodge-plugin-terrain
	PRIVATE
		"src/lodge_terrain_component.c"
		"src/lodge_foliage_component.c"
		"src/lodge_terrain_system.c"
		"src/lodge_plugin_terrain.c"
		"src/lodge_terrain_system.h"
	PUBLIC
		"include/lodge_plugin_terrain.h"
		"include/lodge_terrain_component.h"
		"include/lodge_foliage_component.h"
)

target_include_directories(lodge-plugin-terrain
	PRIVATE
		"src/"
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugin-terrain
	PRIVATE
		lodge-build-flags
		lodge-gfx
		lodge-plugin-editor
	PUBLIC
		lodge-lib
		lodge-entity
		lodge-plugin-scene-renderer
		lodge-plugin-debug-draw
		lodge-plugin-shaders
)

lodge_target_make_plugin(lodge-plugin-terrain "lodge_plugin_terrain.h" lodge_plugin_terrain)