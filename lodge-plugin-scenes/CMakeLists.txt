add_library(lodge-plugin-scenes STATIC "")

target_sources(lodge-plugin-scenes
	PRIVATE
		"src/lodge_plugin_scenes.c"
	PUBLIC
		"include/lodge_plugin_scenes.h"
)

target_include_directories(lodge-plugin-scenes
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugin-scenes
	PRIVATE
		lodge-build-flags
		lodge-assets
	PUBLIC
		lodge-plugins
		lodge-entity
		lodge-plugin-files
)

lodge_target_make_plugin(lodge-plugin-scenes "lodge_plugin_scenes.h" lodge_plugin_scenes)