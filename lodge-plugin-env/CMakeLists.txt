add_library(lodge-plugin-env STATIC "")

target_sources(lodge-plugin-env
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/env.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/env.h"
)

target_include_directories(lodge-plugin-env
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-env
	PRIVATE
		lodge-build-flags
		lodge-plugins
)
