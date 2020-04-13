add_library(lodge-plugins STATIC "")

target_sources(lodge-plugins
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugins.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugins.h"
)

target_include_directories(lodge-plugins
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugins
	PUBLIC
		lodge-lib

		lodge-plugin-vfs		# FIXME(TS): not statically defined
		lodge-window			# FIXME(TS): separate plugin module
		lodge-renderer			# FIXME(TS): separate plugin module
)
