# lodge-plugin-water

add_library(lodge-plugin-water STATIC "")

target_sources(lodge-plugin-water
	PRIVATE
		"src/lodge_water_component.c"
		"src/lodge_water_system.c"
		"src/lodge_plugin_water.c"
	PUBLIC
		"include/lodge_plugin_water.h"
		"include/lodge_water_component.h"
		"include/lodge_water_system.h"
)

lodge_add_module_config(lodge-plugin-water "assets")

target_include_directories(lodge-plugin-water
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugin-water
	PRIVATE
		lodge-build-flags
		lodge-gfx
	PUBLIC
		lodge-lib
		lodge-entity
		lodge-plugin-shaders
		lodge-plugin-scene-renderer
)

lodge_target_make_plugin(lodge-plugin-water "lodge_plugin_water.h" lodge_plugin_water)