add_library(lodge-plugins STATIC "")

target_sources(lodge-plugins
	PRIVATE
		"src/lodge_plugins.c"
		"src/lodge_argv.c"
		"src/lodge_plugin_registry.c"
	PUBLIC
		"include/lodge_plugin.h"
		"include/lodge_plugins.h"
		"include/lodge_argv.h"
)

target_include_directories(lodge-plugins
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugins
	PRIVATE
		lodge-build-flags
		lodge-vfs
	PUBLIC
		lodge-lib
)
