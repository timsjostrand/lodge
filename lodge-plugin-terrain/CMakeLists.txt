# lodge-plugin-terrain

add_library(lodge-plugin-terrain STATIC "")

target_sources(lodge-plugin-terrain
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_terrain_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_foliage_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_terrain_system.c"
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_plugin_terrain.c"
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_terrain_system.h"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_plugin_terrain.h"
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_terrain_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_foliage_component.h"
)

target_include_directories(lodge-plugin-terrain
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(lodge-plugin-terrain
	PRIVATE
		lodge-build-flags
		lodge-gfx
        lodge-plugin-shaders
	PUBLIC
		lodge-lib
		lodge-entity
		lodge-plugin-scene-renderer
		lodge-plugin-debug-draw
)
