# lodge-plugin-types

add_library(lodge-plugin-types STATIC "")

target_sources(lodge-plugin-types
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_types.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_types.h"
)

target_include_directories(lodge-plugin-types
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-types
	PRIVATE
		lodge-build-flags
		lodge-plugins
)
