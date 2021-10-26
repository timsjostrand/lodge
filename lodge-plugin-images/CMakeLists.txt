add_library(lodge-plugin-images STATIC "")

target_sources(lodge-plugin-images
	PRIVATE
		"lodge_plugin_images.c"
	PUBLIC
		"lodge_plugin_images.h"
)

target_include_directories(lodge-plugin-images
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-images
	PRIVATE
		lodge-build-flags
		lodge-assets
	PUBLIC
		lodge-plugins
		lodge-plugin-files
		lodge-image
)

lodge_target_make_plugin(lodge-plugin-images "lodge_plugin_images.h" lodge_plugin_images)