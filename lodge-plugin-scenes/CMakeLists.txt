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
		lodge-plugin-files
	PUBLIC
		lodge-plugins
		lodge-entity
)
