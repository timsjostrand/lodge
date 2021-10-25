#
# cJSON.
#
set(ENABLE_CJSON_TEST OFF CACHE BOOL "Enable building cJSON test" FORCE)
set(ENABLE_CJSON_UNINSTALL OFF CACHE BOOL "Enable creating uninstall target" FORCE)
add_subdirectory(lib/cjson ${CMAKE_BINARY_DIR}/lib/cjson)

#
# lodge-serialize-json
#
add_library(lodge-serialize-json STATIC "")

target_sources(lodge-serialize-json
	PRIVATE
		"lodge_json.c"
		"lodge_serialize_json.c"
	PUBLIC
		"lodge_json.h"
		"lodge_serialize_json.h"
)

target_include_directories(lodge-serialize-json
	PRIVATE
		"lib/"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-serialize-json
	PRIVATE
		lodge-build-flags
		cjson
		lodge-assets
	PUBLIC
		lodge-lib
)
