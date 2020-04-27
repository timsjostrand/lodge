# lodge-vfs

add_library(lodge-vfs STATIC "")

target_sources(lodge-vfs
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/vfs.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/vfs.h"
)

target_include_directories(lodge-vfs
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-vfs
	PRIVATE
		lodge-build-flags
		lodge-stb
	PUBLIC
		lodge-lib
)
