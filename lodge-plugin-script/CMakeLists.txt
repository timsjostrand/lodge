# lodge-plugin-script

add_library(lodge-plugin-script STATIC "")

target_sources(lodge-plugin-script
	PRIVATE
		"src/lodge_graphs.c"
		"src/lodge_graphs.h"
		"src/lodge_script_component.c"
		"src/lodge_script_component.h"
		"src/lodge_script_system.c"
		"src/lodge_script_system.h"
		"src/lodge_plugin_script.c"
	PUBLIC
		"include/lodge_plugin_script.h"
)

target_include_directories(lodge-plugin-script
	PUBLIC
		"include/"
)

target_link_libraries(lodge-plugin-script
	PRIVATE
		lodge-build-flags
		lodge-assets
	PUBLIC
		lodge-lib
		lodge-plugins
		lodge-entity
		lodge-ns

		lodge-plugin-scene-renderer # FIXME(TS): only for debugging
		lodge-ns-editor # FIXME(TS): ns-editor should hook into editor, not the other way around
		lodge-plugin-editor # FIXME(TS): ns-editor should hook into editor, not the other way around
)
