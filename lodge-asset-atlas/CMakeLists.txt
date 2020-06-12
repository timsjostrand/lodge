# lodge-atlas

add_library(lodge-asset-atlas STATIC "")

target_sources(lodge-asset-atlas
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/atlas.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/atlas.h"
)

target_include_directories(lodge-asset-atlas
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-asset-atlas
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-lib
		cjson
)
