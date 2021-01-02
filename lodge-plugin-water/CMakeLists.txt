# lodge-plugin-water

add_library(lodge-plugin-water STATIC "")

target_sources(lodge-plugin-water
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_water_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_water_system.c"
		"${CMAKE_CURRENT_LIST_DIR}/src/lodge_plugin_water.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_plugin_water.h"
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_water_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/include/lodge_water_system.h"
)

target_include_directories(lodge-plugin-water
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(lodge-plugin-water
	PRIVATE
		lodge-build-flags
		lodge-gfx
        lodge-plugin-shaders
	PUBLIC
		lodge-lib
		lodge-entity
		lodge-plugin-scene-renderer
)
