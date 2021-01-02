#include "lodge_water_system.h"

#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_drawable.h"
#include "lodge_gfx.h"
#include "lodge_assets.h"
#include "lodge_sampler.h"

#include "lodge_water_component.h"
#include "lodge_transform_component.h"
#include "lodge_directional_light_component.h"
#include "lodge_shadow_map.h"

#include "lodge_plugin_water.h"
#include "lodge_plugin_scene_renderer.h"

#include "drawable.h"

#include "geometry.h"
#include "frustum.h"

#include <stdbool.h>

struct lodge_water_system
{
	bool					draw;
	vec3					terrain_scale;

	lodge_shader_t			shader;

	struct drawable			drawable;

	lodge_texture_t			texture_heightmap;
	lodge_texture_t			texture_cubemap;
	lodge_texture_t			texture_water_foam;
	lodge_texture_t			texture_water_normals[2];

	lodge_sampler_t			sampler_linear_repeat;
	lodge_sampler_t			sampler_linear_clamp;
};

static void lodge_water_system_render(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, struct lodge_water_system *system)
{
	if(!system->draw) {
		return;
	}

	ASSERT(pass_params->pass == LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT);

	lodge_gfx_annotate_begin(strview_static("water"));

	lodge_shader_t shader = system->shader;

	lodge_gfx_bind_shader(shader);

	const vec3 directional_light = lodge_directional_light_get_dir(scene);

	//
	// globals
	//
	lodge_shader_set_constant_float(shader, strview_static("time"), pass_params->time);
	lodge_shader_set_constant_vec3(shader, strview_static("terrain_scale"), system->terrain_scale);
	lodge_shader_set_constant_vec3(shader, strview_static("sun_dir"), directional_light);

	//
	// shadow map
	//
	struct lodge_shadow_map *shadow_map = pass_params->data.forward_transparent.shadow_map;
	lodge_gfx_bind_texture_unit_2d_array(7, shadow_map->depth_textures_array, system->sampler_linear_clamp);
	lodge_shader_bind_constant_buffer(shader, 1, shadow_map->buffer_object);

	//
	// distance fog
	//
	lodge_shader_bind_constant_buffer(shader, 2, pass_params->data.forward_transparent.distance_fog);

	lodge_gfx_bind_texture_unit_2d(3, pass_params->data.forward_transparent.depth, system->sampler_linear_repeat);
	lodge_gfx_bind_texture_unit_2d(4, pass_params->data.forward_transparent.color, system->sampler_linear_repeat);
	lodge_gfx_bind_texture_unit_3d(8, pass_params->data.forward_transparent.volumetric_light, system->sampler_linear_repeat);

	if(system->texture_heightmap) {
		lodge_gfx_bind_texture_unit_2d(0, system->texture_heightmap, system->sampler_linear_clamp);
	}

	if(system->texture_cubemap) {
		lodge_gfx_bind_texture_unit_cube_map(1, system->texture_cubemap, system->sampler_linear_repeat);
	}

	if(system->texture_water_foam) {
		lodge_gfx_bind_texture_unit_2d(2, system->texture_water_foam, system->sampler_linear_repeat);
	}

	if(system->texture_water_normals[0]) {
		lodge_gfx_bind_texture_unit_2d(5, system->texture_water_normals[0], system->sampler_linear_repeat);
	}

	if(system->texture_water_normals[1]) {
		lodge_gfx_bind_texture_unit_2d(6, system->texture_water_normals[1], system->sampler_linear_repeat);
	}

	lodge_scene_components_foreach(scene, struct lodge_water_component*, water, LODGE_COMPONENT_TYPE_WATER) {
		lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_WATER, water);

		const vec3 scale = lodge_get_scale(scene, owner);
		const mat4 transform = lodge_get_transform(scene, owner);

		lodge_shader_set_constant_vec3(shader, strview_static("scale"), scale);
		lodge_shader_set_constant_vec3(shader, strview_static("wave_scale"), water->wave_scale);
		lodge_shader_set_constant_vec3(shader, strview_static("water_color"), water->color);
		lodge_shader_set_constant_float(shader, strview_static("water_max_depth"), water->max_depth);

		struct mvp mvp = {
			.model = transform,
			.view = pass_params->camera.view,
			.projection = pass_params->camera.projection
		};

		lodge_shader_set_constant_mvp(shader, &mvp);

		drawable_render(&system->drawable);
	}

	lodge_gfx_annotate_end();
}

static void lodge_water_system_new_inplace(struct lodge_water_system *system, lodge_scene_t scene)
{
	system->draw = true;

	// FIXME(TS): dynamically figure this out -- support multiple terrains
	// entity hierarchy maybe?
	system->terrain_scale = vec3_make(2000.0f, 2000.0f, 300.0f);

	const vec2 origin = { -0.5f, -0.5f };
	const vec2 size = { 1.0f, 1.0f };
	system->drawable = drawable_make_plane_subdivided(origin, size, 1, 1);

	system->sampler_linear_repeat = lodge_sampler_make((struct lodge_sampler_desc) {
		.min_filter = MIN_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_REPEAT,
		.wrap_y = WRAP_REPEAT,
		.wrap_z = WRAP_REPEAT,
	});
	system->sampler_linear_clamp = lodge_sampler_make((struct lodge_sampler_desc) {
		.min_filter = MIN_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE,
	});

	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT, &lodge_water_system_render, system);
}

static void lodge_water_system_free_inplace(struct lodge_water_system *system)
{
	//lodge_scene_render_system_remove_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT, lodge_water_system_render, system);
}

static void lodge_water_system_update(struct lodge_water_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt)
{
	struct lodge_plugin_water *plugin = lodge_system_type_get_plugin(type);
	ASSERT_OR(plugin) {
		return;
	}

	// FIXME(TS): hardcoded file paths

	if(!system->shader) {
		system->shader = (const lodge_shader_t)lodge_assets_get(plugin->shaders, strview_static("water"));
	}

	if(!system->texture_cubemap) {
		system->texture_cubemap = lodge_texture_cubemap_make((struct lodge_texture_cubemap_desc) {
			.back = lodge_assets_get(plugin->images, strview_static("skybox/skybox_back.bmp")),
			.front = lodge_assets_get(plugin->images, strview_static("skybox/skybox_front.bmp")),
			.top = lodge_assets_get(plugin->images, strview_static("skybox/skybox_top.bmp")),
			.bottom = lodge_assets_get(plugin->images, strview_static("skybox/skybox_bottom.bmp")),
			.left = lodge_assets_get(plugin->images, strview_static("skybox/skybox_left.bmp")),
			.right = lodge_assets_get(plugin->images, strview_static("skybox/skybox_right.bmp")),
		});
	}

	if(!system->texture_heightmap) {
		system->texture_heightmap = *(const lodge_texture_t*)lodge_assets_get(plugin->textures, strview_static("heightmap.raw"));
		ASSERT(system->texture_heightmap);
	}

	if(!system->texture_water_foam) {
		system->texture_water_foam = *(const lodge_texture_t*)lodge_assets_get(plugin->textures, strview_static("water_foam.png"));
		ASSERT(system->texture_water_foam);
	}

	if(!system->texture_water_normals[0]) {
		system->texture_water_normals[0] = *(const lodge_texture_t*)lodge_assets_get(plugin->textures, strview_static("water_normals1.png"));
		ASSERT(system->texture_water_normals[0]);
	}

	if(!system->texture_water_normals[1]) {
		system->texture_water_normals[1] = *(const lodge_texture_t*)lodge_assets_get(plugin->textures, strview_static("water_normals2.png"));
		ASSERT(system->texture_water_normals[1]);
	}
}

lodge_system_type_t lodge_water_system_type_register(struct lodge_plugin_water *plugin)
{
	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview_static("water_system"),
		.size = sizeof(struct lodge_water_system),
		.new_inplace = lodge_water_system_new_inplace,
		.free_inplace = NULL,
		.update = lodge_water_system_update,
		.plugin = plugin,
		.properties = {
			.count = 2,
			.elements = {
				{
					.name = strview_static("draw"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_water_system, draw),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
				{
					.name = strview_static("terrain_scale"),
					.type = LODGE_TYPE_VEC3,
					.offset = offsetof(struct lodge_water_system, terrain_scale),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
			}
		}
	});
}
