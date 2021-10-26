add_library(lodge-plugins STATIC "")

target_sources(lodge-plugins
	PRIVATE
		"src/lodge_plugins.c"
		"src/lodge_argv.c"
		"src/lodge_plugin_registry.c"
	PUBLIC
		"include/lodge_plugin.h"
		"include/lodge_plugins.h"
		"include/lodge_argv.h"
)

target_include_directories(lodge-plugins
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugins
	PRIVATE
		lodge-build-flags

		lodge-plugin-vfs					# FIXME(TS): not statically defined
		lodge-plugin-types					# FIXME(TS): not statically defined
		lodge-plugin-images					# FIXME(TS): not statically defined
		lodge-plugin-textures				# FIXME(TS): not statically defined
		lodge-plugin-fbx					# FIXME(TS): not statically defined
		lodge-plugin-shaders				# FIXME(TS): not statically defined
		lodge-plugin-scene-renderer			# FIXME(TS): not statically defined
		lodge-plugin-debug-draw				# FIXME(TS): not statically defined
	PUBLIC
		lodge-lib
		lodge-window						# FIXME(TS): separate plugin module
		lodge-gfx							# FIXME(TS): separate plugin module
)
