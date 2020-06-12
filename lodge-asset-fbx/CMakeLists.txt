add_library(lodge-asset-fbx STATIC "")

target_sources(lodge-asset-fbx
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/fbx_asset.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_res_fbx.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/fbx_asset.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_res_fbx.h"
)

target_include_directories(lodge-asset-fbx
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-asset-fbx
	PRIVATE
		lodge-build-flags
		${GLEW_LIBRARY}
		lodge-collections
		lodge-stb
		lodge-res
	PUBLIC
		lodge-plugins
		lodge-fbx
		lodge-lib
		lodge-renderer
)
