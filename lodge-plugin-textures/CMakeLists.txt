add_library(lodge-plugin-textures STATIC "")

target_sources(lodge-plugin-textures
	PRIVATE
		"lodge_plugin_textures.c"
	PUBLIC
		"lodge_plugin_textures.h"
)

target_include_directories(lodge-plugin-textures
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-textures
	PRIVATE
		lodge-build-flags
		lodge-assets
		lodge-image
		lodge-gfx
	PUBLIC
		lodge-plugins
		lodge-plugin-images
)

lodge_target_make_plugin(lodge-plugin-textures "lodge_plugin_textures.h" lodge_plugin_textures)