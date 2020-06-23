add_library(lodge-plugin-files STATIC "")

target_sources(lodge-plugin-files
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_files.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_files.h"
)

target_include_directories(lodge-plugin-files
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-files
	PRIVATE
		lodge-build-flags
		lodge-res
	PUBLIC
		lodge-plugins
		lodge-vfs
)
