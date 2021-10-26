# lodge-plugin-renderdoc

add_library(lodge-plugin-renderdoc STATIC "")

target_sources(lodge-plugin-renderdoc
	PRIVATE
		"src/renderdoc_app.h"
		"src/lodge_plugin_renderdoc.c"
	PUBLIC
		"include/lodge_plugin_renderdoc.h"
)

target_include_directories(lodge-plugin-renderdoc
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugin-renderdoc
	PRIVATE
		lodge-build-flags
		lodge-plugins
)
