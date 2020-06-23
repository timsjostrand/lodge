add_library(lodge-env STATIC "")

target_sources(lodge-env
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/env.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/env.h"
)

target_include_directories(lodge-env
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-env
	PRIVATE
		lodge-build-flags
		lodge-lib
)
