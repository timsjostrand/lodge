add_library(lodge-plugin-images STATIC "")

target_sources(lodge-plugin-images
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_images.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_images.h"
)

target_include_directories(lodge-plugin-images
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-images
	PRIVATE
		lodge-build-flags
		lodge-res
	PUBLIC
		lodge-plugins
		lodge-image
)
