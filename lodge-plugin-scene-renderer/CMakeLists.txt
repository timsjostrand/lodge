add_library(lodge-plugin-scene-renderer STATIC "")

target_sources(lodge-plugin-scene-renderer
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_static_mesh_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_camera_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_directional_light_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_point_light_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_scene_renderer.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_geometry_buffer.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_post_process.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_shadow_map.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_tesselated_plane.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_billboard_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_billboard_system.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_static_mesh_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_camera_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_directional_light_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_point_light_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_scene_renderer.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_geometry_buffer.h" # FIXME(TS): private later
		"${CMAKE_CURRENT_LIST_DIR}/lodge_post_process.h" # FIXME(TS): private later
		"${CMAKE_CURRENT_LIST_DIR}/lodge_shadow_map.h" # FIXME(TS): private later
		"${CMAKE_CURRENT_LIST_DIR}/lodge_tesselated_plane.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_billboard_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_billboard_system.h"
)

target_include_directories(lodge-plugin-scene-renderer
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-scene-renderer
	PRIVATE
		lodge-build-flags
		lodge-plugin-editor
	PUBLIC
		lodge-plugins
		lodge-plugin-fbx
		lodge-plugin-shaders
		lodge-plugin-textures
		lodge-entity
)
