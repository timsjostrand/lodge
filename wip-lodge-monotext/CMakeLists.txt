add_library(lodge-monotext STATIC "")

target_sources(lodge-monotext
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/monofont.c"
		"${CMAKE_CURRENT_LIST_DIR}/monotext.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/monofont.h"
		"${CMAKE_CURRENT_LIST_DIR}/monotext.h"
)

target_include_directories(lodge-monotext
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-monotext
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-lib
		lodge-gfx
)
