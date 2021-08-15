add_library(lodge-plugin-textures STATIC "")

target_sources(lodge-plugin-textures
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_textures.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_textures.h"
)

target_include_directories(lodge-plugin-textures
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-textures
	PRIVATE
		lodge-build-flags
		lodge-assets
		lodge-image
		lodge-gfx
		lodge-plugin-images # TODO(TS): PUBLIC instead?
	PUBLIC
		lodge-plugins
)
