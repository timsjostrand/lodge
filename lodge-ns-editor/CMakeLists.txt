# lodge-ns-editor

add_library(lodge-ns-editor STATIC "")

target_sources(lodge-ns-editor
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_editor.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_editor.h"
)

target_include_directories(lodge-ns-editor
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-ns-editor
	PRIVATE
		lodge-build-flags
		lodge-gui
	PUBLIC
		lodge-lib
		lodge-ns
)
