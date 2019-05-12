add_library(lodge-asset-fbx STATIC "")

target_sources(lodge-asset-fbx
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/fbx.c"
		#"${CMAKE_CURRENT_LIST_DIR}/fbx_asset.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/fbx.h"
		#"${CMAKE_CURRENT_LIST_DIR}/fbx_asset.h"
)

target_include_directories(lodge-asset-fbx
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-asset-fbx
	PUBLIC
		"lodge-collections"
		"lodge-stb"
)
