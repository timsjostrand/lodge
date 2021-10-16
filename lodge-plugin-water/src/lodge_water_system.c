#include "lodge_water_system.h"

#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_drawable.h"
#include "lodge_gfx.h"
#include "lodge_assets2.h"
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

struct lodge_water_render_data
{
	lodge_shader_t			shader;
	lodge_texture_t			foam;
	lodge_texture_t			wave_normals[2];
};

struct lodge_water_system
{
	bool					draw;
	vec3					terrain_scale;

	struct drawable			drawable;

	struct lodge_assets2	*shaders;
	struct lodge_assets2	*textures;

	lodge_texture_t			texture_heightmap;
	lodge_texture_t			texture_cubemap;

	lodge_sampler_t			sampler_linear_repeat;
	lodge_sampler_t			sampler_linear_clamp;

	lodge_asset_t			shader_asset;
};

static void lodge_water_system_render(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, struct lodge_water_system *system)
{
	if(!system->draw) {
		return;
	}

	ASSERT_OR(pass_params->pass == LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT) { return; }

	lodge_shader_t shader = lodge_assets2_get(system->shaders, system->shader_asset);
	ASSERT_OR(shader) { return; }

	lodge_gfx_annotate_begin(strview("water"));

	lodge_gfx_bind_shader(shader);

	const vec3 directional_light = lodge_directional_light_get_dir(scene);

	//
	// globals
	//
	lodge_shader_set_constant_float(shader, strview("time"), pass_params->time);
	lodge_shader_set_constant_vec3(shader, strview("terrain_scale"), system->terrain_scale);
	lodge_shader_set_constant_vec3(shader, strview("sun_dir"), directional_light);

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

	lodge_scene_components_foreach(scene, struct lodge_water_component*, water, LODGE_COMPONENT_TYPE_WATER) {
		lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_WATER, water);

		lodge_texture_t *texture_water_foam = lodge_assets2_get(system->textures, water->foam_asset);
		lodge_texture_t *texture_water_normals[2] = {
			lodge_assets2_get(system->textures, water->normals_1_asset),
			lodge_assets2_get(system->textures, water->normals_2_asset),
		};

		lodge_gfx_annotate_begin(lodge_scene_get_entity_name(scene, owner));

		if(texture_water_foam) {
			lodge_gfx_bind_texture_unit_2d(2, *texture_water_foam, system->sampler_linear_repeat);
		}
		if(texture_water_normals[0]) {
			lodge_gfx_bind_texture_unit_2d(5, *texture_water_normals[0], system->sampler_linear_repeat);
		}
		if(texture_water_normals[1]) {
			lodge_gfx_bind_texture_unit_2d(6, *texture_water_normals[1], system->sampler_linear_repeat);
		}

		const vec3 scale = lodge_get_scale(scene, owner);
		const mat4 transform = lodge_get_transform(scene, owner);

		lodge_shader_set_constant_vec3(shader, strview("scale"), scale);
		lodge_shader_set_constant_vec3(shader, strview("wave_scale"), water->wave_scale);
		lodge_shader_set_constant_vec3(shader, strview("water_color"), water->color);
		lodge_shader_set_constant_float(shader, strview("water_max_depth"), water->max_depth);

		struct mvp mvp = {
			.model = transform,
			.view = pass_params->camera.view,
			.projection = pass_params->camera.projection
		};

		lodge_shader_set_constant_mvp(shader, &mvp);

		drawable_render(&system->drawable);

		lodge_gfx_annotate_end();
	}

	lodge_gfx_annotate_end();
}

static void lodge_water_system_new_inplace(struct lodge_water_system *system, lodge_scene_t scene, struct lodge_plugin_water *plugin)
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

	system->shaders = plugin->shaders;
	system->textures = plugin->textures;
	system->shader_asset = lodge_assets2_register(plugin->shaders, strview("plugins/water/water"));

	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT, &lodge_water_system_render, system);
}

static void lodge_water_system_free_inplace(struct lodge_water_system *system, struct lodge_plugin_water *plugin)
{
	//lodge_scene_render_system_remove_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT, lodge_water_system_render, system);
}

static void lodge_water_system_update(struct lodge_water_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_plugin_water *plugin)
{
	//
	// FIXME(TS): proper support for skybox assets through asset manager
	//
	if(!system->texture_cubemap) {
		lodge_asset_t assets[] = {
			lodge_assets2_register(plugin->images, strview("skybox/skybox_back.bmp")),
			lodge_assets2_register(plugin->images, strview("skybox/skybox_front.bmp")),
			lodge_assets2_register(plugin->images, strview("skybox/skybox_top.bmp")),
			lodge_assets2_register(plugin->images, strview("skybox/skybox_bottom.bmp")),
			lodge_assets2_register(plugin->images, strview("skybox/skybox_left.bmp")),
			lodge_assets2_register(plugin->images, strview("skybox/skybox_right.bmp")),
		};

		system->texture_cubemap = lodge_texture_cubemap_make((struct lodge_texture_cubemap_desc) {
			.back = lodge_assets2_get(plugin->images, assets[0]),
			.front = lodge_assets2_get(plugin->images, assets[1]),
			.top = lodge_assets2_get(plugin->images, assets[2]),
			.bottom = lodge_assets2_get(plugin->images, assets[3]),
			.left = lodge_assets2_get(plugin->images, assets[4]),
			.right = lodge_assets2_get(plugin->images, assets[5]),
		});
	}

	//
	// FIXME(TS): heightmap should be fetched implicitly from a sibling terrain component instead.
	//
	if(!system->texture_heightmap) {
		lodge_asset_t asset = lodge_assets2_register(plugin->textures, strview("heightmap.raw"));
		system->texture_heightmap = *(lodge_texture_t*)lodge_assets2_get(plugin->textures, asset);
		ASSERT(system->texture_heightmap);
	}
}

lodge_system_type_t lodge_water_system_type_register(struct lodge_plugin_water *plugin, lodge_type_t shader_asset_type)
{
	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview("water_system"),
		.size = sizeof(struct lodge_water_system),
		.new_inplace = lodge_water_system_new_inplace,
		.free_inplace = NULL,
		.update = lodge_water_system_update,
		.plugin = plugin,
		.properties = {
			.count = 3,
			.elements = {
				{
					.name = strview("draw"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_water_system, draw),
				},
				{
					.name = strview("terrain_scale"),
					.type = LODGE_TYPE_VEC3,
					.offset = offsetof(struct lodge_water_system, terrain_scale),
				},
				{
					.name = strview("shader"),
					.type = shader_asset_type,
					.offset = offsetof(struct lodge_water_system, shader_asset),
				}
			}
		}
	});
}

