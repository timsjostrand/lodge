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

#set_target_properties(lodge-plugin-renderdoc PROPERTIES
#	LINK_FLAGS "/INCLUDE:lodge_plugin_renderdoc /INCLUDE:lodge_plugin_renderdoc_static"
#)

target_link_libraries(lodge-plugin-renderdoc
	PRIVATE
		lodge-build-flags
		lodge-plugins
)

lodge_target_make_plugin(lodge-plugin-renderdoc "lodge_plugin_renderdoc.h" lodge_plugin_renderdoc)