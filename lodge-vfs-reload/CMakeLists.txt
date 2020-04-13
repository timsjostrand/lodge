# lodge-vfs-reload

#
# FIXME(TS): this entire module is just providing legacy behavior
#

add_library(lodge-vfs-reload STATIC "")

target_sources(lodge-vfs-reload
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/util_reload.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/util_reload.h"
)

target_include_directories(lodge-vfs-reload
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-vfs-reload
	PUBLIC
		lodge-lib
		lodge-vfs
		lodge-image
		lodge-renderer
		lodge-sound
)
