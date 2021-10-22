add_library(lodge-plugin-scene-renderer STATIC "")

target_sources(lodge-plugin-scene-renderer
	PRIVATE
		"lodge_static_mesh_component.c"
		"lodge_directional_light_component.c"
		"lodge_point_light_component.c"
		"lodge_plugin_scene_renderer.c"
		"lodge_geometry_buffer.c"
		"lodge_post_process.c"
		"lodge_shadow_map.c"
		"lodge_tesselated_plane.c"
		"lodge_billboard_component.c"
		"lodge_billboard_system.c"
	PUBLIC
		"lodge_static_mesh_component.h"
		"lodge_directional_light_component.h"
		"lodge_point_light_component.h"
		"lodge_plugin_scene_renderer.h"
		"lodge_geometry_buffer.h"
		"lodge_post_process.h"
		"lodge_shadow_map.h"
		"lodge_tesselated_plane.h"
		"lodge_billboard_component.h"
		"lodge_billboard_system.h"
)

target_include_directories(lodge-plugin-scene-renderer
	PUBLIC
		"./"
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

lodge_target_make_plugin(lodge-plugin-scene-renderer "lodge_plugin_scene_renderer.h" lodge_scene_renderer_plugin)