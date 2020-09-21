# lodge-vfs

add_library(lodge-vfs STATIC "")

target_sources(lodge-vfs
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_vfs.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_filewatch.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_vfs.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_filewatch.h"
)

target_include_directories(lodge-vfs
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
