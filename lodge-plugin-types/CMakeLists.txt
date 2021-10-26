# lodge-plugin-types

add_library(lodge-plugin-types STATIC "")

target_sources(lodge-plugin-types
	PRIVATE
		"lodge_plugin_types.c"
	PUBLIC
		"lodge_plugin_types.h"
)

set_target_properties(lodge-plugin-types PROPERTIES
	LINKER_LANGUAGE C
)

target_include_directories(lodge-plugin-types
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-types
	PRIVATE
		lodge-build-flags
		lodge-lib
		lodge-serialize-json
	PUBLIC
		lodge-plugins
)

lodge_target_make_plugin(lodge-plugin-types "lodge_plugin_types.h" lodge_plugin_types)

#set_property(
#	TARGET lodge-plugin-types
#	PROPERTY STATIC_LIBRARY_OPTIONS
#	"/INCLUDE:lodge_plugin_types /INCLUDE:lodge_plugin_types_register /export:lodge_plugin_types /export:lodge_plugin_types_register"
#)

#set_target_properties(lodge-plugin-types PROPERTIES
#	LINK_FLAGS "/INCLUDE:lodge_plugin_types /INCLUDE:lodge_plugin_types_register /export:lodge_plugin_types /export:lodge_plugin_types_register"
#)
