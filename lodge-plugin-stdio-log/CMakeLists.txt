add_library(lodge-plugin-stdio-log STATIC "")

target_sources(lodge-plugin-stdio-log
	PRIVATE
		"lodge_plugin_stdio_log.c"
	PUBLIC
		"lodge_plugin_stdio_log.h"
)

target_include_directories(lodge-plugin-stdio-log
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-stdio-log
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-plugins
)

lodge_target_make_plugin(lodge-plugin-stdio-log "lodge_plugin_stdio_log.h" lodge_plugin_stdio_log)