add_library(lodge-plugin-fbx STATIC "")

target_sources(lodge-plugin-fbx
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/fbx_asset.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_fbx.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/fbx_asset.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_fbx.h"
)

target_include_directories(lodge-plugin-fbx
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-fbx
	PRIVATE
		lodge-build-flags
		${GLEW_LIBRARY}
		lodge-collections
		lodge-stb
		lodge-res
	PUBLIC
		lodge-plugins
		lodge-plugin-files
		lodge-fbx
		lodge-lib
		lodge-renderer
)
