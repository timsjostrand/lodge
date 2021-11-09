
add_library(lodge-plugin-shaders STATIC "")

target_sources(lodge-plugin-shaders
	PRIVATE
		"lodge_plugin_shaders.c"
		"lodge_plugin_shader_sources.c"
	PUBLIC
		"lodge_plugin_shaders.h"
		"lodge_plugin_shader_sources.h"
)

target_include_directories(lodge-plugin-shaders
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-shaders
	PRIVATE
		lodge-build-flags
		lodge-plugins
		lodge-gfx
    PUBLIC
		lodge-assets
		lodge-plugin-files
)

lodge_add_module_config(lodge-plugin-shaders "assets")

lodge_target_make_plugin(lodge-plugin-shaders "lodge_plugin_shaders.h" lodge_plugin_shaders)
lodge_target_make_plugin(lodge-plugin-shaders "lodge_plugin_shader_sources.h" lodge_plugin_shader_sources)