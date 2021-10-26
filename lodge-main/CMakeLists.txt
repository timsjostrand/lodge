add_library(lodge-main STATIC "")

target_sources(lodge-main
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/main.c"
)

target_include_directories(lodge-main
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-main
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-plugins
)
