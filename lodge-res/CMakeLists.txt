# lodge-res
#
# Framework for hotswappable resource manager with dependency graph.
#

add_library(lodge-res STATIC "")

target_sources(lodge-res
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_res.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_res.h"
)

target_include_directories(lodge-res
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-res
	PRIVATE
		lodge-build-flags
		lodge-plugins
)
