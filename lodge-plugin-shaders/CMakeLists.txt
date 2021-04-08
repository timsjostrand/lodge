
add_library(lodge-plugin-shaders STATIC "")

target_sources(lodge-plugin-shaders
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_shaders.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_shader_sources.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_shaders.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_shader_sources.h"
)

target_include_directories(lodge-plugin-shaders
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-shaders
	PRIVATE
		lodge-plugins
		lodge-gfx
		lodge-plugin-files
    PUBLIC
        lodge-assets
)
