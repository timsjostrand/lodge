add_library(lodge-plugins STATIC "")

target_sources(lodge-plugins
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugins.c"
		"${CMAKE_CURRENT_LIST_DIR}/core_argv.c" 		# FIXME(TS): deprecate
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugins.h"
		"${CMAKE_CURRENT_LIST_DIR}/core_argv.h"			# FIXME(TS): deprecate
		"${CMAKE_CURRENT_LIST_DIR}/game.h"				# FIXME(TS): hack
)

target_include_directories(lodge-plugins
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugins
	PRIVATE
		lodge-build-flags

		lodge-plugin-vfs					# FIXME(TS): not statically defined
		lodge-plugin-types					# FIXME(TS): not statically defined
		lodge-plugin-env					# FIXME(TS): not statically defined
		lodge-plugin-images					# FIXME(TS): not statically defined
		lodge-plugin-textures				# FIXME(TS): not statically defined
		lodge-plugin-fbx					# FIXME(TS): not statically defined
		lodge-plugin-shaders				# FIXME(TS): not statically defined
		lodge-plugin-scene-renderer			# FIXME(TS): not statically defined
		lodge-plugin-debug-draw				# FIXME(TS): not statically defined
		lodge-plugin-terrain				# FIXME(TS): not statically defined
		lodge-plugin-water					# FIXME(TS): not statically defined
	PUBLIC
		lodge-lib
		lodge-window						# FIXME(TS): separate plugin module
		lodge-renderer						# FIXME(TS): separate plugin module
		lodge-res							# FIXME(TS): for lodge_res_files_plugin()
)
