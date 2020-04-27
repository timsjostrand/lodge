# lodge-image

add_library(lodge-image STATIC "")

target_sources(lodge-image
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_image.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_image.h"
)

target_include_directories(lodge-image
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-image
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-lib
		lodge-stb
)
