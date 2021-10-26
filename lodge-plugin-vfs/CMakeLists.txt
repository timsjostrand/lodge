# lodge-plugin-vfs

add_library(lodge-plugin-vfs STATIC "")

target_sources(lodge-plugin-vfs
	PRIVATE
		"lodge_plugin_vfs.c"
	PUBLIC
		"lodge_plugin_vfs.h"
)

target_include_directories(lodge-plugin-vfs
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-vfs
	PRIVATE
		lodge-build-flags
		lodge-plugins
		lodge-vfs
)

lodge_target_make_plugin(lodge-plugin-vfs "lodge_plugin_vfs.h" lodge_plugin_vfs)
