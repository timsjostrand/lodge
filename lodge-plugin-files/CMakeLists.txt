add_library(lodge-plugin-files STATIC "")

target_sources(lodge-plugin-files
	PRIVATE
		"lodge_plugin_files.c"
	PUBLIC
		"lodge_plugin_files.h"
)

target_include_directories(lodge-plugin-files
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-files
	PRIVATE
		lodge-build-flags
		lodge-assets
	PUBLIC
		lodge-plugins
		lodge-vfs
)

lodge_target_make_plugin(lodge-plugin-files "lodge_plugin_files.h" lodge_plugin_files)