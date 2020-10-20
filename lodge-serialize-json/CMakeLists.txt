# lodge-serialize-json

add_library(lodge-serialize-json STATIC "")

target_sources(lodge-serialize-json
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_json.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_serialize_json.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_json.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_serialize_json.h"
)

target_include_directories(lodge-serialize-json
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-serialize-json
	PRIVATE
		lodge-build-flags
		cjson
	PUBLIC
		lodge-lib
)
