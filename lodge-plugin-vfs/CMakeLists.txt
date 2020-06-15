# lodge-plugin-vfs

add_library(lodge-plugin-vfs STATIC "")

target_sources(lodge-plugin-vfs
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_vfs.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_vfs.h"
)

target_include_directories(lodge-plugin-vfs
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-vfs
	PRIVATE
		lodge-build-flags
		lodge-plugins
		lodge-vfs
)
