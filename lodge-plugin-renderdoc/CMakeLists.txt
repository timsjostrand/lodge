# lodge-plugin-renderdoc

add_library(lodge-plugin-renderdoc STATIC "")

target_sources(lodge-plugin-renderdoc
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/renderdoc_app.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_renderdoc.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_renderdoc.h"
)

target_include_directories(lodge-plugin-renderdoc
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-renderdoc
	PRIVATE
		lodge-build-flags
		lodge-plugins
)
