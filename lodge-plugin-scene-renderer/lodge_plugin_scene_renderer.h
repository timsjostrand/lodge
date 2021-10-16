#ifndef _LODGE_PLUGIN_SCENE_RENDERER_H
#define _LODGE_PLUGIN_SCENE_RENDERER_H

#include "lodge_plugin.h"

#include "math4.h"

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_scene_renderer_plugin;
struct lodge_scene_render_system;

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_geometry_buffer;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

enum lodge_scene_render_system_pass
{
	LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED = 0,
	LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD,
	LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT,
	LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW,
	LODGE_SCENE_RENDER_SYSTEM_PASS_POST_PROCESS,
	LODGE_SCENE_RENDER_SYSTEM_PASS_MAX,
};

//
// TODO(TS): use alignas() instead of manual padding
//
struct lodge_camera_params
{
	vec3								pos;
	float								_pad;
	//vec3								dir;
	mat4								view;
	mat4								projection;
	mat4								view_projection;
	mat4								inv_view;
	mat4								inv_projection;
	mat4								inv_view_projection;
	float								fov_y;
	mat4								rotation;
};

struct lodge_scene_render_pass_params
{
	enum lodge_scene_render_system_pass	pass;

	float								time;

	struct lodge_camera_params			camera;
	lodge_buffer_object_t				camera_buffer;

	union
	{
		struct lodge_geometry_buffer	*shadow;
		struct lodge_geometry_buffer	*deferred;
		struct
		{
			lodge_texture_t				color;
			lodge_texture_t				depth;
		}								forward;
		struct
		{
			lodge_texture_t				color;
			lodge_texture_t				depth;
			struct lodge_shadow_map		*shadow_map;
			lodge_buffer_object_t		distance_fog;
			lodge_texture_t				volumetric_light;
		}								forward_transparent;
		struct lodge_geometry_buffer	*post_process;
	}									data;
};

struct lodge_scene_renderer_types
{
	lodge_system_type_t				scene_render_system;

	lodge_system_type_t				billboard_system;
	lodge_component_type_t			billboard_component;

	lodge_component_type_t			camera_component_type;
	lodge_component_type_t			point_light_component_type;
	lodge_component_type_t			directional_light_component_type;
	lodge_component_type_t			static_mesh_component_type;
};

typedef void						(*lodge_scene_render_system_func_t)(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, void *userdata);

struct lodge_plugin_desc			lodge_scene_renderer_plugin();
struct lodge_scene_renderer_types	lodge_scene_renderer_plugin_get_types(struct lodge_scene_renderer_plugin *plugin);
struct lodge_assets2*				lodge_scene_renderer_plugin_get_shaders(struct lodge_scene_renderer_plugin *plugin);
struct lodge_assets2*				lodge_scene_renderer_plugin_get_textures(struct lodge_scene_renderer_plugin *plugin);

void								lodge_scene_add_render_pass_func(lodge_scene_t scene, enum lodge_scene_render_system_pass pass, lodge_scene_render_system_func_t func, void *userdata);
void								lodge_scene_remove_render_pass_func(lodge_scene_t scene, enum lodge_scene_render_system_pass pass, lodge_scene_render_system_func_t func, void *userdata);

//
// TODO(TS): should this be passed as argument to _render() instead?
//
void								lodge_scene_set_active_camera(lodge_scene_t scene, lodge_entity_t camera_entity);
lodge_entity_t						lodge_scene_get_entity_at_screen_pos(lodge_scene_t scene, vec2 screen_pos);

#endif