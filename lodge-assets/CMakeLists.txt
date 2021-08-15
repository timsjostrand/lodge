# lodge-assets
#
# Framework for hotswappable resource manager with dependency graph.
#

add_library(lodge-assets STATIC "")

target_sources(lodge-assets
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_assets.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_assets.h"
)

target_include_directories(lodge-assets
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-assets
	PRIVATE
		lodge-build-flags
		lodge-plugins
)
