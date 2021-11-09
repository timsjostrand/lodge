#include "lodge_terrain_system.h"

#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_drawable.h"
#include "lodge_gfx.h"
#include "lodge_assets2.h"
#include "lodge_sampler.h"
#include "lodge_debug_draw.h"
#include "lodge_buffer_object.h"

#include "lodge_terrain_component.h"
#include "lodge_foliage_component.h"
#include "lodge_transform_component.h"

#include "lodge_editor_selection_system.h"

#include "lodge_plugin_terrain.h"
#include "lodge_plugin_scene_renderer.h"
#include "lodge_plugin_debug_draw.h"

#include "drawable.h" // FIXME(TS): port to lodge_drawable

#include "dynbuf.h"
#include "geometry.h"
#include "frustum.h"
#include "gruvbox.h"

#include <stdbool.h>

struct terrain_render_data
{
	vec3							scale;
	lodge_texture_t					heightmap;
	lodge_texture_t					albedo;
	lodge_texture_t					normal;
	lodge_texture_t					displacement;
	lodge_texture_t					metalness;
};

struct terrain_render_datas
{
	size_t							count;
	size_t							capacity;
	struct terrain_render_data		*elements;

	lodge_shader_t					terrain_shader;
	lodge_shader_t					foliage_shader;
};

struct lodge_terrain_system
{
	bool							draw;
	bool							update_lod_cull;
	bool							tesselated_plane;
	bool							debug;
	
	enum lodge_terrain_lod_level	lod_level_min;
	enum lodge_terrain_lod_level	lod_level_max;
	float							lod_switch_threshold; // if size in clip space >= threshold, switch to higher res lod

	struct drawable					lod_meshes[LODGE_TERRAIN_LOD_LEVEL_MAX];
	struct drawable					centered_lod_meshes[LODGE_TERRAIN_LOD_LEVEL_MAX];

	lodge_asset_t					terrain_shader_asset;
	lodge_asset_t					foliage_shader_asset;
	lodge_asset_t					terrain_entity_id_shader_asset;
	lodge_asset_t					foliage_entity_id_shader_asset;

	lodge_sampler_t					material_sampler;
	lodge_sampler_t					heightfield_sampler;

	struct terrain_render_datas		render_datas;

	struct lodge_debug_draw			*debug_draw;
};

static int lodge_terrain_chunk_in_frustum(struct lodge_terrain_chunk *chunk, struct frustum_planes *frustum /*, struct lodge_debug_draw *debug_draw*/)
{
	struct aabb aabb = {
		.min = chunk->world_offset,
		.max = vec3_add(chunk->world_offset, chunk->world_size)
	};

#if 0
	if(debug_draw) {
		lodge_debug_draw_aabb_outline(debug_draw, aabb, COLOR_GREEN, 0.0f);
	}
#endif

	return frustum_planes_vs_aabb(frustum, aabb);
}

static void lodge_terrain_system_update_lod_levels(struct lodge_terrain_system *system, struct lodge_terrain_component *terrain, const vec3 scale, const vec3 camera_pos)
{
	const float lod_dist_max = 999.0f;
	const float lod_dist_min = 10.0f;

	for(uint32_t y = 0, y_max = terrain->chunks_y; y<y_max; y++) {
		for(uint32_t x=0, x_max = terrain->chunks_x; x<x_max; x++) {
			struct lodge_terrain_chunk *chunk = lodge_terrain_component_get_chunk(terrain, x, y);

			vec3 chunk_center = chunk->world_offset;
			chunk_center.x += terrain->chunk_size.x / 2.0f * scale.x;
			chunk_center.y += terrain->chunk_size.y / 2.0f * scale.y;
			chunk_center.z = camera_pos.z;
			const float dist = vec3_distance(camera_pos, chunk_center);

			const float lod_factor = clamp(dist, lod_dist_min, lod_dist_max) / (lod_dist_max - lod_dist_min);
			chunk->lod = lerp1f(system->lod_level_max, system->lod_level_min, lod_factor);
		}
	}
}

static void lodge_terrain_system_update_visible_chunks(struct lodge_terrain_component *component, const vec3 pos, const vec3 scale, struct frustum_planes *frustum)
{
	for(uint32_t y = 0, y_max = component->chunks_y; y<y_max; y++) {
		for(uint32_t x=0, x_max = component->chunks_x; x<x_max; x++) {
			struct lodge_terrain_chunk *chunk = lodge_terrain_component_get_chunk(component, x, y);

			chunk->world_offset.x = pos.x + chunk->offset.x * scale.x;
			chunk->world_offset.y = pos.y + chunk->offset.y * scale.y;
			chunk->world_offset.z = pos.z;

			chunk->world_size = vec3_mult(scale, component->chunk_size);

			chunk->visible = lodge_terrain_chunk_in_frustum(chunk, frustum /*, params->debug_draw*/);
		}
	}
}



static void lodge_terrain_system_render(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, struct lodge_terrain_system *system)
{
	if(!system->draw) {
		return;
	}

	//
	// Cull+LOD update
	//
	if(system->update_lod_cull) {
		struct frustum_planes frustum = frustum_planes_make(pass_params->camera.view_projection);

		lodge_scene_components_foreach(scene, struct lodge_terrain_component*, component, LODGE_COMPONENT_TYPE_TERRAIN) {
			lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_TERRAIN, component);
			const vec3 owner_pos = lodge_get_position(scene, owner);
			const vec3 owner_scale = lodge_get_scale(scene, owner);

			if(!system->tesselated_plane) {
				lodge_terrain_system_update_visible_chunks(component, owner_pos, owner_scale, &frustum);

				//
				// In the shadow pass, re-use the LOD levels selected for the main camera pass, since updating
				// the LODs would cause severe shadow aliasing in the lighting calculation.
				//
				if(pass_params->pass != LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW) {
					lodge_terrain_system_update_lod_levels(system, component, owner_scale, pass_params->camera.pos);
				}
			} else {
				if(pass_params->pass != LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW) {
					lodge_tesselated_plane_update(component->plane,
						owner_pos,
						owner_scale,
						&pass_params->camera,
						system->lod_level_min,
						system->lod_switch_threshold,
						system->debug ? system->debug_draw : NULL
					);
				}
			}
		}
	}

	lodge_shader_t terrain_shader = NULL;
	lodge_shader_t foliage_shader = NULL;
		ASSERT_OR(system->render_datas.terrain_shader && system->render_datas.foliage_shader) { return; }
		terrain_shader = system->render_datas.terrain_shader;
		foliage_shader = system->render_datas.foliage_shader;

	//
	// Render
	//
	{
		lodge_gfx_annotate_begin(strview("terrain"));

		lodge_gfx_bind_shader(terrain_shader);

		lodge_shader_set_constant_float(terrain_shader, strview("time"), pass_params->time);
		
		//
		// FIXME(TS): do once
		//
		lodge_shader_bind_constant_buffer(terrain_shader, 0, pass_params->camera_buffer);

		int i = 0;
		lodge_scene_components_foreach(scene, struct lodge_terrain_component*, component, LODGE_COMPONENT_TYPE_TERRAIN) {
			struct terrain_render_data *render_data = &system->render_datas.elements[i++];

			lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_TERRAIN, component);

			lodge_shader_set_constant_vec3(terrain_shader, strview("terrain_scale"), render_data->scale);
			lodge_gfx_bind_texture_unit_2d(0, render_data->heightmap, system->heightfield_sampler);
			lodge_gfx_bind_texture_unit_2d(1, render_data->albedo, system->material_sampler);
			lodge_gfx_bind_texture_unit_2d(2, render_data->displacement, system->material_sampler);
			lodge_gfx_bind_texture_unit_2d(3, render_data->normal, system->material_sampler);
			lodge_gfx_bind_texture_unit_2d(4, render_data->metalness, system->material_sampler);

			lodge_shader_set_constant_vec2(terrain_shader, strview("chunk_size"), vec2_make(xy_of(component->chunk_size)));

#if 1
			lodge_shader_set_constant_float(terrain_shader, strview("entity_id"), (float)((uintptr_t)owner));
			const bool selected = lodge_scene_is_entity_selected(scene, owner);
			lodge_shader_set_constant_float(terrain_shader, strview("entity_selected"), selected ? 1.0f : 0.0f);
#endif

			if(!system->tesselated_plane) {
				for(int y=0, y_max=component->chunks_y; y<y_max; y++) {
					for(int x=0, x_max=component->chunks_x; x<x_max; x++) {
						struct lodge_terrain_chunk *chunk = lodge_terrain_component_get_chunk(component, x, y);

						if(!chunk->visible) {
							continue;
						}

						mat4 model = mat4_translation(xyz_of(chunk->world_offset));
						//model = mat4_rotate_z(model, time/10000.0f);
						model = mat4_scale(model, xyz_of(chunk->world_size));

						lodge_shader_set_constant_vec2(terrain_shader, strview("chunk_offset"), vec2_make(x/(float)x_max, y/(float)y_max));
						lodge_shader_set_constant_mat4(terrain_shader, strview("model"), model);

						int lod_level = (int)chunk->lod;
						ASSERT(lod_level >= LODGE_TERRAIN_LOD_LEVEL_128 && lod_level < LODGE_TERRAIN_LOD_LEVEL_MAX);
						struct drawable *chunk_lod = &system->lod_meshes[lod_level];

						drawable_render(chunk_lod);
					}
				}
			} else {
				for(struct lodge_tesselated_plane_chunk *it = lodge_tesselated_plane_chunks_begin(component->plane),
					*end = lodge_tesselated_plane_chunks_end(component->plane); it < end; it++) {
					
					mat4 model = mat4_translation(it->center.x, it->center.y, 0.0f);
					model = mat4_scale(model, it->size.x, it->size.y, render_data->scale.z); // FIXME(TS): Z scaling

					lodge_shader_set_constant_mat4(terrain_shader, strview("model"), model);
					lodge_shader_set_constant_float(terrain_shader, strview("chunk_level"), (float)it->lod);

					ASSERT(it->lod >= 0 && it->lod < LODGE_TERRAIN_LOD_LEVEL_MAX);
					struct drawable *chunk_lod = &system->centered_lod_meshes[it->lod];
					drawable_render(chunk_lod);
				}
			}

			struct lodge_foliage_component *foliage = lodge_scene_get_entity_component(scene, owner, LODGE_COMPONENT_TYPE_FOLIAGE);
			if(foliage) {
				lodge_foliage_system_render(foliage, pass_params, foliage_shader, system->heightfield_sampler, component, owner, render_data->scale);
			}
		}

		lodge_gfx_annotate_end();
	}
}

static void lodge_terrain_system_new_inplace(struct lodge_terrain_system *system, lodge_scene_t scene, struct lodge_plugin_terrain *plugin)
{
	system->draw = true;
	system->update_lod_cull = true;
	system->tesselated_plane = true;
	system->lod_level_min = LODGE_TERRAIN_LOD_LEVEL_1;
	system->lod_level_max = LODGE_TERRAIN_LOD_LEVEL_128;
	system->lod_switch_threshold = 1.5f;

	system->terrain_shader_asset = lodge_assets2_register(plugin->shaders, strview("terrain"));
	system->foliage_shader_asset = lodge_assets2_register(plugin->shaders, strview("foliage_rocks"));
	system->terrain_entity_id_shader_asset = lodge_assets2_register(plugin->shaders, strview("terrain_id"));
	system->foliage_entity_id_shader_asset = lodge_assets2_register(plugin->shaders, strview("foliage_rocks_id"));

	dynbuf_new_inplace(dynbuf(system->render_datas), 8);

	const int lod_level_to_subdivisions[LODGE_TERRAIN_LOD_LEVEL_MAX] = {
		[LODGE_TERRAIN_LOD_LEVEL_128] =	128,
		[LODGE_TERRAIN_LOD_LEVEL_64] =	 64,
		[LODGE_TERRAIN_LOD_LEVEL_32] =	 32,
		[LODGE_TERRAIN_LOD_LEVEL_16] =	 16,
		[LODGE_TERRAIN_LOD_LEVEL_8] =	  8,
		[LODGE_TERRAIN_LOD_LEVEL_4] =	  4,
		[LODGE_TERRAIN_LOD_LEVEL_2] =	  2,
		[LODGE_TERRAIN_LOD_LEVEL_1] =	  1,
	};

	system->heightfield_sampler = lodge_sampler_make((struct lodge_sampler_desc) {
		.min_filter = MIN_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE,
	});

	system->material_sampler = lodge_sampler_make((struct lodge_sampler_desc) {
		.min_filter = MIN_FILTER_LINEAR_MIPMAP_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_REPEAT,
		.wrap_y = WRAP_REPEAT,
		.wrap_z = WRAP_REPEAT,
	});

	//
	// Make reusable chunks
	//
	for(int i=0; i<LODGE_TERRAIN_LOD_LEVEL_MAX; i++) {
		int subdivisions = lod_level_to_subdivisions[i];
		system->lod_meshes[i] = drawable_make_plane_subdivided_vertex(vec2_make(0.0f, 0.0f), vec2_make(1.0f, 1.0f), subdivisions, subdivisions);
		system->centered_lod_meshes[i] = drawable_make_plane_subdivided_vertex(vec2_make(-0.5f,-0.5f), vec2_make(1.0f,1.0f), subdivisions, subdivisions);
	}

	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED, &lodge_terrain_system_render, system);
	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW, &lodge_terrain_system_render, system);
}

static void lodge_terrain_system_free_inplace(struct lodge_terrain_system *system, lodge_scene_t scene, struct lodge_plugin_terrain *plugin)
{
	//lodge_scene_render_system_remove_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED, lodge_terrain_system_render, system);
	//lodge_scene_render_system_remove_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW, lodge_terrain_system_render, system);
	
	for(int i=0; i<LODGE_TERRAIN_LOD_LEVEL_MAX; i++) {
		drawable_reset(&system->lod_meshes[i]);
		drawable_reset(&system->centered_lod_meshes[i]);
	}
}

static void lodge_terrain_system_update(struct lodge_terrain_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_plugin_terrain *plugin)
{
	if(system->terrain_shader_asset) {
		system->render_datas.terrain_shader = lodge_assets2_get(plugin->shaders, system->terrain_shader_asset);
	}
	if(system->foliage_shader_asset) {
		system->render_datas.foliage_shader = lodge_assets2_get(plugin->shaders, system->foliage_shader_asset);
	}

	//
	// FIXME(TS): helper for this?
	//
	lodge_system_type_t debug_draw_system_type = lodge_plugin_debug_draw_get_system_type(plugin->plugin_debug_draw);
	if(debug_draw_system_type) {
		struct lodge_debug_draw_system *debug_draw_system = lodge_scene_get_system(scene, debug_draw_system_type);
		system->debug_draw = debug_draw_system ? lodge_debug_draw_system_get_batcher(debug_draw_system) : NULL;
	} else {
		system->debug_draw = NULL;
	}

	struct lodge_assets2 *textures = plugin->textures;
	ASSERT_OR(textures) {
		return;
	}

	dynbuf_clear(dynbuf(system->render_datas));
	lodge_scene_components_foreach(scene, struct lodge_terrain_component*, component, LODGE_COMPONENT_TYPE_TERRAIN) {
		lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_TERRAIN, component);

		const vec3 scale = lodge_get_scale(scene, owner);

		lodge_texture_t *heightmap = lodge_assets2_get(textures, component->heightmap);
		lodge_texture_t *albedo = lodge_assets2_get(textures, component->material.albedo);
		lodge_texture_t *displacement = lodge_assets2_get(textures, component->material.displacement);
		lodge_texture_t *normal = lodge_assets2_get(textures, component->material.normal);
		lodge_texture_t *metalness = lodge_assets2_get(textures, component->material.metalness);

		if(heightmap && albedo && displacement && normal && metalness) {
			struct terrain_render_data *render_data = dynbuf_append_no_init(dynbuf(system->render_datas));
			render_data->scale = scale;
			render_data->heightmap = *heightmap;
			render_data->albedo = *albedo;
			render_data->displacement = *displacement;
			render_data->normal = *normal;
			render_data->metalness = *metalness;
		}
	}
}

static void on_modified_lod_level(struct lodge_property *property, const struct lodge_terrain_system *system)
{
	ASSERT(system->lod_level_max <= system->lod_level_min);
}

lodge_system_type_t lodge_terrain_system_type_register(struct lodge_plugin_terrain *plugin)
{
	//
	// The lod level represents the number of subdivisions; double them for UI purposes.
	//
	lodge_type_t lodge_type_enum_terrain_lod_level = lodge_type_register_enum(
		strview("terrain_lod_level"),
		(struct lodge_enum_desc) {
			.count = LODGE_TERRAIN_LOD_LEVEL_MAX,
			.elements = {
				{
					.name = strview("256x256"),
					.value = LODGE_TERRAIN_LOD_LEVEL_128,
				},
				{
					.name = strview("128x128"),
					.value = LODGE_TERRAIN_LOD_LEVEL_64,
				},
				{
					.name = strview("64x64"),
					.value = LODGE_TERRAIN_LOD_LEVEL_32,
				},
				{
					.name = strview("32x32"),
					.value = LODGE_TERRAIN_LOD_LEVEL_16,
				},
				{
					.name = strview("16x16"),
					.value = LODGE_TERRAIN_LOD_LEVEL_8,
				},
				{
					.name = strview("8x8"),
					.value = LODGE_TERRAIN_LOD_LEVEL_4,
				},
				{
					.name = strview("4x4"),
					.value = LODGE_TERRAIN_LOD_LEVEL_2,
				},
				{
					.name = strview("2x2"),
					.value = LODGE_TERRAIN_LOD_LEVEL_1,
				},
			},
		}
	);

	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview("terrain_system"),
		.size = sizeof(struct lodge_terrain_system),
		.new_inplace = &lodge_terrain_system_new_inplace,
		.free_inplace = &lodge_terrain_system_free_inplace,
		.update = &lodge_terrain_system_update,
		.userdata = plugin,
		.properties = {
			.count = 7,
			.elements = {
				{
					.name = strview("draw"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_terrain_system, draw),
				},
				{
					.name = strview("update_lod_cull"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_terrain_system, update_lod_cull),
				},
				{
					.name = strview("tesselated_plane"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_terrain_system, tesselated_plane),
				},
				{
					.name = strview("debug"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_terrain_system, debug),
				},
				{
					.name = strview("lod_level_min"),
					.type = lodge_type_enum_terrain_lod_level,
					.offset = offsetof(struct lodge_terrain_system, lod_level_min),
					.on_modified = &on_modified_lod_level,
				},
				{
					.name = strview("lod_level_max"),
					.type = lodge_type_enum_terrain_lod_level,
					.offset = offsetof(struct lodge_terrain_system, lod_level_max),
					.on_modified = &on_modified_lod_level,
				},
				{
					.name = strview("lod_switch_threshold"),
					.type = LODGE_TYPE_F32,
					.offset = offsetof(struct lodge_terrain_system, lod_switch_threshold),
				},
			}
		}
	});
}
