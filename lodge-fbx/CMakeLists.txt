add_library(lodge-fbx STATIC "")

target_sources(lodge-fbx
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/fbx.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/fbx.h"
)

target_include_directories(lodge-fbx
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-fbx
	PRIVATE
		lodge-build-flags
		lodge-stb
		lodge-collections
		lodge-lib
)
