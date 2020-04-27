# lodge-console

add_library(lodge-console STATIC "")

target_sources(lodge-console
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/console.c"
		"${CMAKE_CURRENT_LIST_DIR}/core_console.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/console.h"
		"${CMAKE_CURRENT_LIST_DIR}/core_console.h"
)

target_include_directories(lodge-console
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-console
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-sprite
		lodge-monotext
		lodge-plugin-env
)
