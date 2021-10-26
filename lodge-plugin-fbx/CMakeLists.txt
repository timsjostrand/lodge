add_library(lodge-plugin-fbx STATIC "")

target_sources(lodge-plugin-fbx
	PRIVATE
		"fbx_asset.c"
		"lodge_plugin_fbx.c"
	PUBLIC
		"fbx_asset.h"
		"lodge_plugin_fbx.h"
)

target_include_directories(lodge-plugin-fbx
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-fbx
	PRIVATE
		lodge-build-flags
		lodge-collections
		lodge-stb
		lodge-assets
		lodge-lib
	PUBLIC
		lodge-plugins
		lodge-plugin-files
		lodge-fbx
		lodge-gfx
)

lodge_target_make_plugin(lodge-plugin-fbx "lodge_plugin_fbx.h" lodge_plugin_fbx)