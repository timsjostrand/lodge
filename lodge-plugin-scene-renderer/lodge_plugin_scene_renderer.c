#include "lodge_plugin_scene_renderer.h"

#include "math4.h"
#include "membuf.h"
#include "fbx_asset.h"
#include "gruvbox.h"

#include "lodge_plugin.h"
#include "lodge_plugins.h"
#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_assets2.h"
#include "lodge_window.h"
#include "lodge_bound_func.h"

#include "lodge_static_mesh_component.h"
#include "lodge_transform_component.h"
#include "lodge_camera_component.h"
#include "lodge_directional_light_component.h"
#include "lodge_point_light_component.h"

#include "lodge_billboard_component.h"
#include "lodge_billboard_system.h"

#include "lodge_perspective.h"
#include "lodge_gfx.h"
#include "lodge_pipeline.h"
#include "lodge_framebuffer.h"
#include "lodge_geometry_buffer.h"
#include "lodge_post_process.h"
#include "lodge_sampler.h"
#include "lodge_shadow_map.h"
#include "lodge_texture.h"
#include "lodge_buffer_object.h"
#include "lodge_debug_draw.h"
#include "lodge_shader.h"

#include "lodge_plugin_fbx.h"
#include "lodge_plugin_textures.h"
#include "lodge_plugin_shaders.h"

#include "lodge_editor_selection_system.h"

#include <stdio.h>
#define alignas _Alignas

struct lodge_scene_renderer_plugin
{
	struct lodge_assets2				*fbx_assets;
	struct lodge_assets2				*shaders;
	struct lodge_assets2				*textures;

	lodge_system_type_t					scene_render_system_type;

	lodge_component_type_t				billboard_component_type;
	lodge_system_type_t					billboard_system_type;

	lodge_component_type_t				point_light_component_type;
	lodge_component_type_t				directional_light_component_type;
	lodge_component_type_t				static_mesh_component_type;
};

struct lodge_render_system_pass
{
	lodge_scene_render_system_func_t	funcs[256];
	void								*func_userdatas[256];
	size_t								funcs_count;
};

struct lodge_distance_fog
{
	alignas(16) vec3					color;
	alignas(16) vec3					sun_color;
	alignas(4) float					density;
};

struct lodge_transform_uniform
{
	alignas(16) mat4					model;

	//
	// UBOs need to be aligned to at least `UNIFORM_BUFFER_OFFSET_ALIGNMENT`
	//
	mat4								_pad0;
	mat4								_pad1;
	mat4								_pad2;
};

struct lodge_static_meshes
{
	bool								draw;
	struct lodge_assets2				*shaders;
	struct lodge_assets2				*textures;

	size_t								count;
	struct lodge_static_mesh_component	*components[1024];
	struct lodge_transform_uniform		transforms[1024];
	const struct fbx_asset				*meshes[1024];
	lodge_entity_t						ids[1024];
	float								selected[1024];

	lodge_buffer_object_t				transforms_buffer;
};

struct lodge_hdr_resolve
{
	alignas(4) float					gamma;
	alignas(4) float					exposure;
	alignas(4) float					bloom_intensity;
};

struct lodge_light
{
	alignas(16) vec4					pos_dir;							// if .w==0 => direction, otherwise position
	alignas(16) vec4					intensity_attenuation;				// rgb = intensity, a = attenuation
	alignas(16) vec4					cone_direction_ambient_coefficient; // rgb = cone_direction, a = ambient_coefficient
	alignas(16) vec4					cone_angle;
};

struct lodge_lights
{
	alignas(16) struct lodge_light		elements[128];
	alignas(4) int32_t					count;
};

struct lodge_hdr
{
	lodge_asset_t						hdr_extract_shader;
	lodge_texture_t						bloom_downsample_texture;
	lodge_texture_t						bloom_downsample_texture_levels[16];
	lodge_texture_t						bloom_upsample_texture;
	lodge_texture_t						bloom_upsample_texture_levels[16];
	uint32_t							desired_bloom_samples_count;
	uint32_t							bloom_samples_count;

	bool								bloom_enable;
	lodge_asset_t						bloom_downsample_shader;
	lodge_asset_t						bloom_upsample_shader;

	struct lodge_hdr_resolve			hdr_resolve;
	lodge_asset_t						hdr_resolve_shader;
	lodge_texture_t						hdr_resolve_texture;
	lodge_buffer_object_t				hdr_resolve_buffer_object;
};

struct lodge_scene_render_system
{
	struct lodge_assets2				*shaders;
	struct lodge_assets2				*textures;

	bool								draw_post_process;
	bool								wireframe;

	float								time;

	uint32_t							render_width;
	uint32_t							render_height;

	uint32_t							window_width;
	uint32_t							window_height;

	bool								window_is_render_size;

	lodge_pipeline_t					pipeline_default;
	lodge_pipeline_t					pipeline_wireframe;
	lodge_sampler_t						sampler_linear_clamp;
	lodge_asset_t						shader_deferred_light;

	lodge_buffer_object_t				camera_buffer;

	struct lodge_lights					lights;
	lodge_buffer_object_t				lights_buffer_object;

	lodge_texture_t						texture_offscreen;
	lodge_framebuffer_t					framebuffer_offscreen;

	struct lodge_geometry_buffer		geometry_buffer;
	struct lodge_post_process			post_process;

	struct lodge_shadow_map				shadow_map;
	bool								shadow_map_update;

	lodge_asset_t						volumetric_light_shader;
	lodge_texture_t						volumetric_light_texture;

	struct lodge_hdr					hdr;

	struct lodge_distance_fog			distance_fog;
	lodge_buffer_object_t				distance_fog_buffer;

	//
	// FIXME(TS): port custom render system?
	//
	struct lodge_static_meshes			static_meshes;

	struct lodge_render_system_pass		passes[LODGE_SCENE_RENDER_SYSTEM_PASS_MAX];

	lodge_entity_t						active_camera;
};

lodge_system_type_t LODGE_SYSTEM_TYPE_SCENE_RENDER = NULL;

static void lodge_scene_render_system_post_process_light(const struct lodge_scene_render_system *system, struct mvp mvp, struct lodge_scene_render_pass_params *pass_params)
{
#if 1
	lodge_shader_t deferred_light_shader = lodge_assets2_get(system->shaders, system->shader_deferred_light);

	lodge_gfx_bind_shader(deferred_light_shader);

	//
	// gbuffer
	//
	lodge_gfx_bind_texture_unit_2d(0, system->geometry_buffer.albedo, system->sampler_linear_clamp);
	lodge_gfx_bind_texture_unit_2d(1, system->geometry_buffer.normals, system->sampler_linear_clamp);
	lodge_gfx_bind_texture_unit_2d(2, system->geometry_buffer.depth, system->sampler_linear_clamp);
	lodge_gfx_bind_texture_unit_2d(3, system->geometry_buffer.editor, system->sampler_linear_clamp);

	//
	// shadow map
	//
	lodge_gfx_bind_texture_unit_2d_array(4, system->shadow_map.depth_textures_array, system->sampler_linear_clamp);
	lodge_shader_bind_constant_buffer(deferred_light_shader, 1, system->shadow_map.buffer_object);

	//
	// globals
	//
	lodge_shader_set_constant_vec3(deferred_light_shader, strview("sun_dir"), vec3_make(xyz_of(system->lights.elements[0].pos_dir)));

	//
	// postprocess
	//
	lodge_shader_set_constant_mvp(deferred_light_shader, &mvp);

	//
	// distance_fog
	//
	lodge_shader_bind_constant_buffer(deferred_light_shader, 2, system->distance_fog_buffer);
	lodge_shader_bind_constant_buffer(deferred_light_shader, 4, system->lights_buffer_object);
#endif
}

//
// FIXME(TS): should this live here?
//
static void lodge_static_mesh_render(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, struct lodge_static_meshes *system)
{
	lodge_gfx_annotate_begin(strview("static_meshes"));

	ASSERT_OR(pass_params->pass == LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED
		|| pass_params->pass == LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW) {
		return;
	}

	for(size_t i = 0, count = system->count; i < count; i++) {
		struct lodge_static_mesh_component *component = system->components[i];

		lodge_shader_t shader = NULL;

		//
		// TODO(TS): do asset lookup in _update pass
		//
		shader = lodge_assets2_get(system->shaders, component->shader_asset);

		if(!shader) {
			continue;
		}

		lodge_gfx_bind_shader(shader);

		//
		// TODO(TS): do once in shader init
		//
		lodge_shader_bind_constant_buffer(shader, 0, pass_params->camera_buffer);

		lodge_shader_set_constant_float(shader, strview("entity_id"), (float)((uintptr_t)system->ids[i]));
		lodge_shader_set_constant_float(shader, strview("entity_selected"), (float)((uintptr_t)system->selected[i]));

#if 1
		lodge_shader_bind_constant_buffer_range(shader,
			1,
			system->transforms_buffer,
			i * sizeof(struct lodge_transform_uniform),
			sizeof(struct lodge_transform_uniform)
		);
#else
		//
		// TODO(TS): It would be nice to upload `system->static_mesh_transforms` as offsets into a VBO
		//
		//lodge_shader_set_constant_mat4(static_mesh->shader, strview("model"), system->static_mesh_transforms[i].model);
#endif

		//
		// TODO(TS): do asset lookup in _update pass
		//
		if(component->texture_asset) {
			lodge_texture_t *texture = lodge_assets2_get(system->textures, component->texture_asset);
			if(texture) {
				lodge_gfx_bind_texture_2d(0, *texture);
			}
		}

		lodge_drawable_render_indexed(system->meshes[i]->drawable, system->meshes[i]->static_mesh.indices_count, 0);
	}

	lodge_gfx_annotate_end();
}

static void lodge_static_meshes_new_inplace(struct lodge_static_meshes *static_meshes, lodge_scene_t scene, struct lodge_assets2 *shaders, struct lodge_assets2 *textures)
{
	static_meshes->draw = true;
	static_meshes->count = 0;
	static_meshes->transforms_buffer = lodge_buffer_object_make_dynamic(sizeof(static_meshes->transforms));

	static_meshes->shaders = shaders;
	static_meshes->textures = textures;

	// HACK(TS): static_meshes live here for now FIXME(TS)
	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED, &lodge_static_mesh_render, static_meshes);
	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW, &lodge_static_mesh_render, static_meshes);
}

static void lodge_static_meshes_free_inplace(struct lodge_static_meshes *static_meshes)
{
	lodge_buffer_object_reset(static_meshes->transforms_buffer);
}

struct vec2i
{
	uint32_t x;
	uint32_t y;
};

static struct vec2i lodge_calc_texture_level_size(struct vec2i size, uint32_t level)
{
	const float d = pow(2, level);
	return (struct vec2i) {
		.x = max(floor(size.x / d), 1),
		.y = max(floor(size.y / d), 1),
	};
}

static uint32_t lodge_calc_texture_levels_max(struct vec2i size)
{
	if(size.x <= 0 || size.y <= 0) {
		return 0;
	}

	for(int level = 0; level < 100; level++) {
		const float d = pow(2, level);
		struct vec2i level_size = {
			.x = floor(size.x / d),
			.y = floor(size.y / d),
		};
		if(level_size.x <= 0 || level_size.y <= 0) {
			return level - 1;
		}
	}

	return 0;
}

struct render_size
{
	uint32_t	width;
	uint32_t	height;
	float		aspect_ratio;
};

static struct render_size lodge_scene_render_system_get_render_size(struct lodge_scene_render_system *system)
{
	struct render_size tmp = {
		.width = system->render_width,
		.height = system->render_height,
	};
	if(system->window_is_render_size) {
		tmp.width = system->window_width;
		tmp.height = system->window_height;
	}
	tmp.width = max(tmp.width, 1);
	tmp.height = max(tmp.height, 1);
	tmp.aspect_ratio = tmp.width / (float)tmp.height;
	return tmp;
}


static void lodge_scene_render_system_hdr_resize(struct lodge_hdr *hdr, uint32_t width, uint32_t height)
{
	const uint32_t bloom_levels_max = lodge_calc_texture_levels_max((struct vec2i){ width, height });
	hdr->bloom_samples_count = min(hdr->desired_bloom_samples_count, bloom_levels_max);
	lodge_texture_reset(hdr->bloom_downsample_texture);
	hdr->bloom_downsample_texture = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = hdr->bloom_samples_count,
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA16F,
	});
	lodge_texture_reset(hdr->bloom_upsample_texture);
	hdr->bloom_upsample_texture = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = hdr->bloom_samples_count,
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA16F,
	});
	for(uint32_t i=0; i<hdr->bloom_samples_count; i++) {
		lodge_texture_reset(hdr->bloom_downsample_texture_levels[i]);
		hdr->bloom_downsample_texture_levels[i] = lodge_texture_view_make(hdr->bloom_downsample_texture, LODGE_TEXTURE_TARGET_2D, LODGE_TEXTURE_FORMAT_RGBA16F, i, 1, 0, 1);

		lodge_texture_reset(hdr->bloom_upsample_texture_levels[i]);
		hdr->bloom_upsample_texture_levels[i] = lodge_texture_view_make(hdr->bloom_upsample_texture, LODGE_TEXTURE_TARGET_2D, LODGE_TEXTURE_FORMAT_RGBA16F, i, 1, 0, 1);
	}

	lodge_texture_reset(hdr->hdr_resolve_texture);
	hdr->hdr_resolve_texture = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = 1,
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA16F,
	});
}

static void lodge_hdr_new_inplace(struct lodge_hdr *hdr, struct lodge_assets2 *shaders, struct render_size render_size)
{
	hdr->bloom_enable = true;
	hdr->bloom_samples_count = 7;
	hdr->desired_bloom_samples_count = 7;
	hdr->bloom_downsample_texture = NULL;
	hdr->bloom_upsample_texture = NULL;
	hdr->hdr_resolve_texture = NULL;
	lodge_scene_render_system_hdr_resize(hdr, render_size.width, render_size.height);

	hdr->hdr_extract_shader = lodge_assets2_register(shaders, strview("extract_hdr"));
	hdr->bloom_downsample_shader = lodge_assets2_register(shaders, strview("bloom_downsample"));
	hdr->bloom_upsample_shader = lodge_assets2_register(shaders, strview("bloom_upsample"));

	hdr->hdr_resolve.exposure = 1.0f;
	hdr->hdr_resolve.gamma = 2.2f;
	hdr->hdr_resolve.bloom_intensity = 1.0f;

	hdr->hdr_resolve_shader = lodge_assets2_register(shaders, strview("hdr_resolve"));
	hdr->hdr_resolve_buffer_object = lodge_buffer_object_make_dynamic(sizeof(struct lodge_hdr_resolve));
	lodge_buffer_object_set(hdr->hdr_resolve_buffer_object, 0, &hdr->hdr_resolve, sizeof(struct lodge_hdr_resolve));
}

static void lodge_scene_render_system_offscreen_resized(struct lodge_scene_render_system *system, uint32_t width, uint32_t height)
{
	lodge_texture_reset(system->texture_offscreen);
	system->texture_offscreen = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = 1,
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA16F,
	});

	// FIXME(TS): leaking ->depth?
	lodge_framebuffer_reset(system->framebuffer_offscreen);
	system->framebuffer_offscreen = lodge_framebuffer_make(&(struct lodge_framebuffer_desc) {
		.colors_count = 1,
		.colors = {
			system->texture_offscreen
		},
		.depth = lodge_texture_2d_make_depth(width, height),
		.stencil = NULL,
	});
}

static void lodge_scene_render_system_new_inplace(struct lodge_scene_render_system *system, lodge_scene_t scene, struct lodge_scene_renderer_plugin *plugin)
{
	system->render_width = 1920;
	system->render_height = 1080;
	system->window_width = 1920;
	system->window_height = 1080;
	system->window_is_render_size = true;
	system->draw_post_process = true;
	system->wireframe = false;

	//
	// Pipelines
	//
	{
		struct lodge_pipeline_desc desc = lodge_pipeline_desc_make();

		desc.depth_stencil.depth_compare_func = LODGE_PIPELINE_COMPARE_LEQUAL;
		desc.blend.src_factor_rgb = LODGE_BLEND_FACTOR_SRC_ALPHA;
		desc.blend.dst_factor_rgb = LODGE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		desc.rasterizer.cull_mode = LODGE_RASTERIZER_CULL_MODE_NONE;

		system->pipeline_default = lodge_pipeline_make(desc);

		desc.rasterizer.fill_mode = LODGE_RASTERIZER_FILL_MODE_LINE;
		system->pipeline_wireframe = lodge_pipeline_make(desc);
	}

	// Buffer objects
	{
		system->camera_buffer = lodge_buffer_object_make_dynamic(sizeof(struct lodge_camera_params));
	}

	//
	// Samplers
	//
	{
		system->sampler_linear_clamp = lodge_sampler_make((struct lodge_sampler_desc) {
			.min_filter = MIN_FILTER_LINEAR,
			.mag_filter = MAG_FILTER_LINEAR,
			.wrap_x = WRAP_CLAMP_TO_EDGE,
			.wrap_y = WRAP_CLAMP_TO_EDGE,
			.wrap_z = WRAP_CLAMP_TO_EDGE,
		});
	}

	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	//
	// Off-screen framebuffer
	//
	system->texture_offscreen = NULL;
	system->framebuffer_offscreen = NULL;
	lodge_scene_render_system_offscreen_resized(system, render_size.width, render_size.height);

	//
	// Geometry buffer
	//
	system->geometry_buffer = lodge_geometry_buffer_make(render_size.width, render_size.height);

	//
	// Shadow map
	//
	{
		// FIXME(TS): update from current camera
		const float z_near = 0.1f;
		const float z_far = 10000.0f;
		//const uint32_t width = 1280*5;
		//const uint32_t height = 720*5;
		const uint32_t width = 4096;
		const uint32_t height = 4096;
		lodge_shadow_map_new_inplace(&system->shadow_map, width, height, z_near, z_far);
		system->shadow_map_update = true;
		//system->shadow_map_draw_debug = false;
	}

	system->shaders = plugin->shaders;
	system->textures = plugin->textures;

	//
	// Volumetric light
	//
	{
		system->lights.count = 0;
		system->lights_buffer_object = lodge_buffer_object_make_dynamic(sizeof(struct lodge_lights));
		lodge_buffer_object_set(system->lights_buffer_object, 0, &system->lights, sizeof(system->lights));

		system->volumetric_light_texture = lodge_texture_3d_make((struct lodge_texture_3d_desc) {
			.width = 128,
			.height = 128,
			.depth = 128,
			.texture_format = LODGE_TEXTURE_FORMAT_RGBA32F,
		});
		system->volumetric_light_shader = lodge_assets2_register(plugin->shaders, strview("volumetric_light"));
	}

	//
	// HDR
	//
	lodge_hdr_new_inplace(&system->hdr, plugin->shaders, render_size);

	//
	// Post-processing
	//
	{
		lodge_post_process_new_inplace(&system->post_process, render_size.width, render_size.height, system->geometry_buffer.depth);

		//
		// Deferred lighting
		//
		{
			system->shader_deferred_light = lodge_assets2_register(plugin->shaders, strview("deferred_light"));
			ASSERT(system->shader_deferred_light);
			lodge_post_process_add_func(&system->post_process, lodge_scene_render_system_post_process_light, system);
		}
	}

	//
	// Distance fog
	//
	{
		system->distance_fog.density = 0.002f;
		system->distance_fog.color = vec3_make(0.5f, 0.6f, 0.7f);
		system->distance_fog.sun_color = vec3_make(1.0f, 0.9f, 0.7f);

		system->distance_fog_buffer = lodge_buffer_object_make_dynamic(sizeof(struct lodge_distance_fog));
		lodge_buffer_object_set(system->distance_fog_buffer, 0, &system->distance_fog, sizeof(struct lodge_distance_fog));
	}

	lodge_static_meshes_new_inplace(&system->static_meshes, scene, system->shaders, system->textures);

	{
		struct lodge_scene_funcs *funcs = lodge_scene_get_funcs(scene);
		lodge_bound_func_set(funcs->get_entity_at_screen_pos, system, &lodge_scene_renderer_get_entity_at_screen_pos);
	}
}

void lodge_scene_render_system_free_inplace(struct lodge_scene_render_system *system, struct lodge_scene_renderer_plugin *plugin)
{
	lodge_post_process_free_inplace(&system->post_process);
	lodge_static_meshes_free_inplace(&system->static_meshes);
	lodge_geometry_buffer_reset(&system->geometry_buffer);
	lodge_pipeline_reset(system->pipeline_wireframe);
	lodge_pipeline_reset(system->pipeline_default);
	lodge_buffer_object_reset(system->camera_buffer);
}

//
// TODO(TS): this needs to be done per _render call instead of once in _update (in case of multiple viewports)
//
static void lodge_scene_render_system_update_shadow_map(struct lodge_scene_render_system *system, lodge_scene_t scene, float dt)
{
	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	struct lodge_camera_params camera_params = lodge_camera_params_make(scene, system->active_camera, render_size.aspect_ratio);
	static struct lodge_shadow_map_debug shadow_map_debugs[3];
	if(system->shadow_map_update) {
		lodge_shadow_map_update(
			&system->shadow_map,
			dt,
			camera_params.inv_view_projection,
			vec3_make(xyz_of(system->lights.elements[0].pos_dir)),
			shadow_map_debugs
		);
	}

	//
	// Display shadow map debugging?
	//
#if 0
	if(system->shadow_map_debug_render) {
		for(uint32_t cascade_index = 0; cascade_index < 3; cascade_index++) {
			struct lodge_shadow_map_debug *shadow_map_debug = &shadow_map_debugs[cascade_index];

			lodge_debug_draw_aabb_outline(game->debug_draw, (struct aabb) {
				.min = vec3_make(xyz_of(shadow_map_debug->p_min)),
				.max = vec3_make(xyz_of(shadow_map_debug->p_max)),
			}, GRUVBOX_BRIGHT_BLUE, 0.0f);

			lodge_debug_draw_line(game->debug_draw, (struct line) {
				.p0 = shadow_map_debug->light_pos,
				.p1 = vec3_add(shadow_map_debug->light_pos, vec3_mult_scalar(game->sun_dir, 2000.0f)),
			}, GRUVBOX_BRIGHT_YELLOW, 0.0f);

			lodge_debug_draw_sphere(game->debug_draw, (struct sphere) {
				.pos = shadow_map_debug->light_pos,
				.r = 10.0f
			}, GRUVBOX_BRIGHT_YELLOW, 0.0f);

			lodge_debug_draw_sphere(game->debug_draw, (struct sphere) {
				.pos = shadow_map_debug->frustum_center,
				.r = 0.5f
			}, GRUVBOX_BRIGHT_RED, 0.0f);

			lodge_debug_draw_frustum(game->debug_draw, shadow_map_debug->frustum, GRUVBOX_BRIGHT_GREEN, 0.0f);

			lodge_debug_draw_aabb_outline(game->debug_draw, shadow_map_debug->frustum_bounds, vec4_make(rgb_of(GRUVBOX_BRIGHT_GREEN), 0.5f), 0.0f);
		}
	}
#endif
}

static void lodge_static_meshes_update(struct lodge_static_meshes *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_scene_renderer_plugin *plugin)
{
	// TODO(TS):
	//		- do we want to collect all static_meshes of the same {drawable,material,shader} and instance them?
	//		- maybe separate draw calls is enough?
	//		- maybe we want to just batch them together to make it easier for the driver?

	//
	// Static meshes
	//
	system->count = 0;
	if(system->draw) {
		lodge_scene_components_foreach(scene, struct lodge_static_mesh_component*, static_mesh, LODGE_COMPONENT_TYPE_STATIC_MESH) {
			// TODO: static_mesh->drawable

			// Load FBX?
			const struct fbx_asset *fbx_asset = NULL;
			if(static_mesh->fbx_asset) {
				fbx_asset = lodge_assets2_get(plugin->fbx_assets, static_mesh->fbx_asset);
			}

			// Load shader?
			//if(!static_mesh->shader && !strview_empty(strview_wrap(static_mesh->shader_ref.name))) {
			//	lodge_shader_t shader = (lodge_shader_t)lodge_assets_get(plugin->shaders, strview_wrap(static_mesh->shader_ref.name));
			//	static_mesh->shader = shader;
			//}
			//
			//if(!static_mesh->shader_entity_id) {
			//	lodge_shader_t shader = (lodge_shader_t)lodge_assets_get(plugin->shaders, strview_wrap("static_mesh_id"));
			//	static_mesh->shader_entity_id = shader;
			//}
			//
			//// Load texture?
			//if(!static_mesh->texture && !strview_empty(strview_wrap(static_mesh->texture_ref.name))) {
			//	lodge_texture_t texture = *(lodge_texture_t*)lodge_assets_get(plugin->textures, strview_wrap(static_mesh->texture_ref.name));
			//	static_mesh->texture = texture;
			//}

			//
			// TODO(TS): cull meshes here
			//
			if(fbx_asset
				&& static_mesh->shader_asset
				&& static_mesh->texture_asset) {
				lodge_entity_t entity = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_STATIC_MESH, static_mesh);
				ASSERT(entity);
				if(entity) {
					//
					// TODO(TS): insert sorted, based on the {static_mesh,material,shader} used.
					//
					membuf_append(membuf_wrap(system->components), &system->count, &static_mesh, sizeof(static_mesh));
					system->meshes[system->count - 1] = fbx_asset;
					system->transforms[system->count - 1].model = lodge_get_transform(scene, entity);
					system->ids[system->count - 1] = entity;
					system->selected[system->count - 1] = lodge_scene_is_entity_selected(scene, entity) ? 1.0f : 0.0f;
				}
			}
		}

		if(system->count > 0) {
			lodge_buffer_object_set(
				system->transforms_buffer,
				0,
				&system->transforms,
				system->count * sizeof(struct lodge_transform_uniform)
			);
		}
	}
}

static void lodge_scene_render_system_update(struct lodge_scene_render_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_scene_renderer_plugin *plugin)
{
	system->time += dt;

	//
	// Static meshes -- TODO(TS): move to separate system
	//
	lodge_static_meshes_update(&system->static_meshes, type, scene, dt, plugin);

	//
	// Directional and point lights 
	//
	system->lights.count = 0;
	lodge_scene_components_foreach(scene, struct lodge_directional_light_component*, directional_light, LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT) {
		{
			size_t tmp = system->lights.count;
			struct lodge_light light = {
				.pos_dir = vec4_make_from_vec3(directional_light->dir, 0.0f),
				.intensity_attenuation = vec4_make_from_vec3(directional_light->intensities, 0.0f),
			};
			membuf_append(membuf_wrap(system->lights.elements), &tmp, &light, sizeof(light));
			system->lights.count = tmp;
		}
	}
	lodge_scene_components_foreach(scene, struct lodge_point_light_component*, point_light, LODGE_COMPONENT_TYPE_POINT_LIGHT) {
		{
			size_t tmp = system->lights.count;
			lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_POINT_LIGHT, point_light);

			struct lodge_light light = {
				.pos_dir = vec4_make_from_vec3(lodge_get_position(scene, owner), 1.0f),
				.intensity_attenuation = vec4_make_from_vec3(point_light->intensities, point_light->attenuation),
				.cone_direction_ambient_coefficient = vec4_make_from_vec3(point_light->cone_direction, point_light->ambient_coefficient),
				.cone_angle = vec4_make(point_light->cone_angle, 0.0f, 0.0f, 0.0f),
			};

			membuf_append(membuf_wrap(system->lights.elements), &tmp, &light, sizeof(light));
			system->lights.count = tmp;
		}
	}
	lodge_buffer_object_set(system->lights_buffer_object, 0, &system->lights, sizeof(system->lights));

	//
	// Shadow map (update after: directional lights, camera)
	//
	if(!system->active_camera) {
		lodge_scene_components_foreach(scene, struct lodge_camera_component*, camera, LODGE_COMPONENT_TYPE_CAMERA) {
			if(camera->use_default) {
				lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_CAMERA, camera);
				if(owner) {
					lodge_scene_set_active_camera(scene, owner);
				}
			}
		}
	}

	if(system->active_camera) {
		lodge_scene_render_system_update_shadow_map(system, scene, dt);
	}
}

static void lodge_scene_render_system_render_deferred(struct lodge_scene_render_system *system, lodge_scene_t scene, struct lodge_scene_render_pass_params *pass_params)
{
	lodge_framebuffer_t framebuffer = system->geometry_buffer.framebuffer;

	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	lodge_framebuffer_bind(framebuffer);
	lodge_gfx_set_viewport(0, 0, render_size.width, render_size.height);
	lodge_gfx_set_scissor(0, 0, render_size.width, render_size.height);

	lodge_framebuffer_clear_color(framebuffer, 0, vec4_make(0.33f, 0.33f, 0.33f, 1.0f));
	lodge_framebuffer_clear_color(framebuffer, 1, vec4_zero());
	lodge_framebuffer_clear_color(framebuffer, 2, vec4_zero());
	lodge_framebuffer_clear_depth(framebuffer, LODGE_FRAMEBUFFER_DEPTH_DEFAULT);
	lodge_framebuffer_clear_stencil(framebuffer, LODGE_FRAMEBUFFER_STENCIL_DEFAULT);

	lodge_buffer_object_set(system->camera_buffer, 0, &pass_params->camera, sizeof(struct lodge_camera_params));

	//
	// Pass callbacks
	//
	{
		struct lodge_render_system_pass *deferred_pass = &system->passes[LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED];
		for(size_t i = 0, count = deferred_pass->funcs_count; i < count; i++) {
			deferred_pass->funcs[i](scene, pass_params, deferred_pass->func_userdatas[i]);
		}
	}

	lodge_framebuffer_unbind();
}

static void lodge_scene_render_system_render_forward_transparent(struct lodge_scene_render_system *system, lodge_scene_t scene, struct lodge_scene_render_pass_params *pass_params)
{
	lodge_framebuffer_t framebuffer = system->framebuffer_offscreen;

	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	lodge_framebuffer_bind(framebuffer);
	lodge_gfx_set_viewport(0, 0, render_size.width, render_size.height);
	lodge_gfx_set_scissor(0, 0, render_size.width, render_size.height);

	//lodge_framebuffer_clear_color(framebuffer, 0, vec4_make(0.33f, 0.33f, 0.33f, 1.0f));
	//lodge_framebuffer_clear_depth(framebuffer, LODGE_FRAMEBUFFER_DEPTH_DEFAULT);
	//lodge_framebuffer_clear_stencil(framebuffer, LODGE_FRAMEBUFFER_STENCIL_DEFAULT);

	lodge_buffer_object_set(system->camera_buffer, 0, &pass_params->camera, sizeof(struct lodge_camera_params));

	//
	// Pass callbacks
	//
	{
		struct lodge_render_system_pass *deferred_pass = &system->passes[LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT];
		for(size_t i = 0, count = deferred_pass->funcs_count; i < count; i++) {
			deferred_pass->funcs[i](scene, pass_params, deferred_pass->func_userdatas[i]);
		}
	}

	lodge_framebuffer_unbind();
}

static struct lodge_camera_params lodge_camera_params_make_from_shadow_map_cascade(struct lodge_scene_render_system *system, uint32_t cascade_index)
{
	const mat4 view = system->shadow_map.buffer.views[cascade_index];
	const mat4 projection = system->shadow_map.buffer.projections[cascade_index];
	const mat4 view_projection = mat4_mult(projection, view);

	return (struct lodge_camera_params) {
		.pos = vec3_zero(),
		.view = view,
		.projection = projection,
		.view_projection = view_projection,
		.inv_view = mat4_inverse(view, NULL),
		.inv_projection = mat4_inverse(projection, NULL),
		.inv_view_projection = mat4_inverse(view_projection, NULL),
		.fov_y = 0.0f,
	};
}

static void lodge_scene_render_system_render_shadow_map_cascade(struct lodge_scene_render_system *system, lodge_scene_t scene, uint32_t cascade_index)
{
	char annotation_buf[256];
	strbuf_t annotation = strbuf_wrap(annotation_buf);
	strbuf_setf(annotation, "shadow cascade %u", cascade_index);
	
	lodge_gfx_annotate_begin(strbuf_to_strview(annotation));

	lodge_framebuffer_t framebuffer = system->shadow_map.framebuffer;

	// NOTE(TS): Needs to be done before bind?
	lodge_framebuffer_set_depth_layer(framebuffer, system->shadow_map.depth_textures_array, cascade_index);

	lodge_framebuffer_bind(framebuffer);
	lodge_gfx_set_viewport(0, 0, system->shadow_map.width, system->shadow_map.height);
	lodge_gfx_set_scissor(0, 0, system->shadow_map.width, system->shadow_map.height);

	//lodge_framebuffer_clear_color(framebuffer, 0, vec4_make(0.33f, 0.33f, 0.33f, 1.0f));
	lodge_framebuffer_clear_depth(framebuffer, LODGE_FRAMEBUFFER_DEPTH_DEFAULT);
	lodge_framebuffer_clear_stencil(framebuffer, LODGE_FRAMEBUFFER_STENCIL_DEFAULT);

	struct lodge_scene_render_pass_params pass_params = {
		.pass = LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW,
		.time = system->time,
		.camera = lodge_camera_params_make_from_shadow_map_cascade(system, cascade_index),
		.camera_buffer = system->camera_buffer,
		.data.shadow = &system->geometry_buffer,
	};

	lodge_buffer_object_set(system->camera_buffer, 0, &pass_params.camera, sizeof(struct lodge_camera_params));

	//
	// Pass callbacks
	//
	{
		struct lodge_render_system_pass *shadow_pass = &system->passes[LODGE_SCENE_RENDER_SYSTEM_PASS_SHADOW];
		for(size_t i = 0, count = shadow_pass->funcs_count; i < count; i++) {
			shadow_pass->funcs[i](scene, &pass_params, shadow_pass->func_userdatas[i]);
		}
	}

	lodge_gfx_annotate_end();
}

static void lodge_scene_render_system_copy_deferred_to_offscreen(struct lodge_scene_render_system *system)
{
	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	struct lodge_recti src_rect = {
		.x0 = (int32_t)0,
		.y0 = (int32_t)0,
		.x1 = (int32_t)render_size.width,
		.y1 = (int32_t)render_size.height
	};
	struct lodge_recti dst_rect = {
		.x0 = (int32_t)0,
		.y0 = (int32_t)0,
		.x1 = (int32_t)render_size.width,
		.y1 = (int32_t)render_size.height
	};
	if(system->draw_post_process) {
		lodge_framebuffer_copy(system->framebuffer_offscreen, system->post_process.framebuffer_result, dst_rect, src_rect);
	} else {
		lodge_framebuffer_copy(system->framebuffer_offscreen, system->geometry_buffer.framebuffer, dst_rect, src_rect);
	}
}

static void on_modified_render_size(struct lodge_property *property, struct lodge_scene_render_system *system)
{
	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	lodge_scene_render_system_hdr_resize(&system->hdr, render_size.width, render_size.height);
	lodge_scene_render_system_offscreen_resized(system, render_size.width, render_size.height);
	lodge_geometry_buffer_remake(&system->geometry_buffer, render_size.width, render_size.height);
	lodge_post_process_resize(&system->post_process, render_size.width, render_size.height, system->geometry_buffer.depth);
}

static void on_modified_bloom_levels(struct lodge_property *property, struct lodge_scene_render_system *system)
{
	struct render_size render_size = lodge_scene_render_system_get_render_size(system);
	lodge_scene_render_system_hdr_resize(&system->hdr, render_size.width, render_size.height);
}

static void on_modified_distance_fog(struct lodge_property *property, struct lodge_scene_render_system *system)
{
	lodge_buffer_object_set(system->distance_fog_buffer, 0, &system->distance_fog, sizeof(struct lodge_distance_fog));
}

static void on_modified_hdr_resolve(struct lodge_property *property, struct lodge_scene_render_system *system)
{
	lodge_buffer_object_set(system->hdr.hdr_resolve_buffer_object, 0, &system->hdr.hdr_resolve, sizeof(struct lodge_hdr_resolve));
}

static void lodge_scene_render_system_render(struct lodge_scene_render_system *system, lodge_scene_t scene, struct lodge_system_render_params *render_params, struct lodge_scene_renderer_plugin *plugin)
{
	if(!system->active_camera) {
		return;
	}

	//
	// Check window size
	//
	{
		int window_width = lodge_recti_get_width(&render_params->window_rect);
		int window_height = lodge_recti_get_height(&render_params->window_rect);
		if(window_width != system->window_width || window_height != system->window_height) {
			// FIXME(TS): go through lodge_property_set instead
			system->window_width = window_width;
			system->window_height = window_height;
			on_modified_render_size(NULL, system);
		}
	}

	struct render_size render_size = lodge_scene_render_system_get_render_size(system);

	struct lodge_camera_params camera_params = lodge_camera_params_make(scene, system->active_camera, render_size.aspect_ratio);

	lodge_gfx_annotate_begin(strview("scene_render_system"));
	lodge_pipeline_push(system->wireframe ? system->pipeline_wireframe : system->pipeline_default);

	//
	// Shadow map
	//
	{
		lodge_gfx_annotate_begin(strview("shadow pass"));
		for(uint32_t cascade_index = 0; cascade_index < 3; cascade_index++) {
			lodge_scene_render_system_render_shadow_map_cascade(system, scene, cascade_index);
		}
		lodge_gfx_annotate_end();
	}

	//
	// Deferred rendering
	//
	{
		lodge_gfx_annotate_begin(strview("deferred pass"));
		struct lodge_scene_render_pass_params pass_params = {
			.pass = LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED,
			.time = system->time,
			.camera = camera_params,
			.camera_buffer = system->camera_buffer,
			.data.deferred = &system->geometry_buffer,
		};
		lodge_scene_render_system_render_deferred(system, scene, &pass_params);
		lodge_gfx_annotate_end();
	}

	//
	// Volumetric light
	//
	{
		lodge_shader_t volumetric_light_shader = lodge_assets2_get(system->shaders, system->volumetric_light_shader);

		lodge_gfx_annotate_begin(strview("volumetric light"));
		lodge_gfx_bind_shader(volumetric_light_shader);
		lodge_shader_bind_constant_buffer(volumetric_light_shader, 0, system->camera_buffer);
		lodge_shader_bind_constant_buffer(volumetric_light_shader, 1, system->shadow_map.buffer_object);
		lodge_shader_bind_constant_buffer(volumetric_light_shader, 2, system->distance_fog_buffer);
		lodge_shader_set_constant_vec3(volumetric_light_shader, strview("sun_dir"), lodge_directional_light_get_dir(scene));
		lodge_gfx_bind_texture_unit_2d_array(4, system->shadow_map.depth_textures_array, system->sampler_linear_clamp);
		lodge_gfx_bind_texture_3d_output(0, system->volumetric_light_texture, LODGE_TEXTURE_FORMAT_RGBA32F);
		lodge_gfx_bind_texture_unit_2d(1, system->geometry_buffer.depth, system->sampler_linear_clamp);
		lodge_gfx_bind_texture_unit_2d(2, system->geometry_buffer.albedo, system->sampler_linear_clamp);
		lodge_shader_dispatch_compute(32, 32, 1);
		lodge_gfx_annotate_end();
	}

	//
	// Deferred post-process
	//
	if(system->draw_post_process) {
		lodge_gfx_annotate_begin(strview("post process pass"));
		struct lodge_scene_render_pass_params pass_params = {
			.pass = LODGE_SCENE_RENDER_SYSTEM_PASS_POST_PROCESS,
			.time = system->time,
			.camera = camera_params,
			.camera_buffer = system->camera_buffer,
			.data.post_process = &system->geometry_buffer,
		};
		lodge_post_process_render(&system->post_process, &pass_params);
		lodge_gfx_annotate_end();
	}

	//
	// Texture to screen
	//
	{
		lodge_gfx_annotate_begin(strview("deferred to screen"));
		lodge_scene_render_system_copy_deferred_to_offscreen(system);
		lodge_gfx_annotate_end();
	}

	//
	// TODO(TS): forward passes should also be to off-screen framebuffer and copied to default FB
	//

	//
	// Forward + transparent
	//
	{
		lodge_gfx_annotate_begin(strview("forward-transparent pass"));
		struct lodge_scene_render_pass_params pass_params = {
			.pass = LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT,
			.time = system->time,
			.camera = camera_params,
			.camera_buffer = system->camera_buffer,
			.data.forward_transparent = {
				.color = system->draw_post_process ? system->post_process.color_texture : system->geometry_buffer.albedo,
				.depth = system->geometry_buffer.depth,
				.shadow_map = &system->shadow_map,
				.distance_fog = system->distance_fog_buffer,
				.volumetric_light = system->volumetric_light_texture,
			}
		};
		lodge_scene_render_system_render_forward_transparent(system, scene, &pass_params);
		lodge_gfx_annotate_end();
	}

	// HDR
	{
		struct lodge_hdr *hdr = &system->hdr;

		const uint32_t groups_x = render_size.width / 10;
		const uint32_t groups_y = render_size.height / 10;

		lodge_gfx_annotate_begin(strview("hdr"));
		{
			lodge_shader_t hdr_extract_shader = lodge_assets2_get(system->shaders, hdr->hdr_extract_shader);

			lodge_gfx_annotate_begin(strview("extract_hdr"));
			lodge_gfx_bind_shader(hdr_extract_shader);
			lodge_gfx_bind_texture_2d_output(0, hdr->bloom_downsample_texture, 0, LODGE_TEXTURE_FORMAT_RGBA16F);
			lodge_gfx_bind_texture_unit_2d(1, system->texture_offscreen, system->sampler_linear_clamp);
			lodge_shader_dispatch_compute(groups_x, groups_y, 1);
			lodge_gfx_annotate_end();
		}
		if(hdr->bloom_enable){
			struct vec2i render_size_v2i = { .x = render_size.width, .y = render_size.height };

			const int group_size = 16;

			lodge_gfx_annotate_begin(strview("bloom_downsample"));
			{
				lodge_shader_t bloom_downsample_shader = lodge_assets2_get(system->shaders, hdr->bloom_downsample_shader);
				lodge_gfx_bind_shader(bloom_downsample_shader);
				for(uint32_t dst_mip = 1; dst_mip < hdr->bloom_samples_count; dst_mip++) {
					const uint32_t src_mip = dst_mip - 1;

					struct vec2i level_size_src = lodge_calc_texture_level_size(render_size_v2i, src_mip);
					struct vec2i level_size_dst = lodge_calc_texture_level_size(render_size_v2i, dst_mip);

					lodge_shader_set_constant_float(bloom_downsample_shader, strview("src_level"), src_mip);
					lodge_shader_set_constant_vec2(bloom_downsample_shader, strview("src_size"), vec2_make(xy_of(level_size_src)));
					lodge_shader_set_constant_vec2(bloom_downsample_shader, strview("dst_size"), vec2_make(xy_of(level_size_dst)));
					lodge_gfx_bind_texture_2d_output(0, hdr->bloom_downsample_texture, dst_mip, LODGE_TEXTURE_FORMAT_RGBA16F);
					lodge_gfx_bind_texture_unit_2d(1, hdr->bloom_downsample_texture_levels[src_mip], system->sampler_linear_clamp);
					lodge_shader_dispatch_compute(max(level_size_dst.x/group_size, 1), max(level_size_dst.y/group_size, 1), 1);
				}
			}
			lodge_gfx_annotate_end();

			lodge_gfx_annotate_begin(strview("bloom_upsample"));
			{
				lodge_shader_t bloom_upsample_shader = lodge_assets2_get(system->shaders, hdr->bloom_upsample_shader);
				lodge_gfx_bind_shader(bloom_upsample_shader);
				for(int dst_mip = hdr->bloom_samples_count - 2; dst_mip >= 0; dst_mip--) {
					const uint32_t src_mip = dst_mip + 1;
					struct vec2i level_size_src = lodge_calc_texture_level_size(render_size_v2i, src_mip);
					struct vec2i level_size_dst = lodge_calc_texture_level_size(render_size_v2i, dst_mip);
					lodge_shader_set_constant_float(bloom_upsample_shader, strview("src_level"), src_mip);
					lodge_shader_set_constant_vec2(bloom_upsample_shader, strview("src_size"), vec2_make(xy_of(level_size_src)));
					lodge_shader_set_constant_vec2(bloom_upsample_shader, strview("dst_size"), vec2_make(xy_of(level_size_dst)));
					lodge_gfx_bind_texture_2d_output(0, hdr->bloom_upsample_texture, dst_mip, LODGE_TEXTURE_FORMAT_RGBA16F);
					lodge_gfx_bind_texture_unit_2d(1, hdr->bloom_downsample_texture_levels[src_mip], system->sampler_linear_clamp);
					lodge_shader_dispatch_compute(max(level_size_dst.x/group_size, 1), max(level_size_dst.y/group_size, 1), 1);
				}
			}
			lodge_gfx_annotate_end();
		}
		{
			lodge_shader_t hdr_resolve_shader = lodge_assets2_get(system->shaders, hdr->hdr_resolve_shader);
			lodge_gfx_annotate_begin(strview("hdr_resolve"));
			lodge_gfx_bind_shader(hdr_resolve_shader);
			lodge_gfx_bind_texture_2d_output(0, system->texture_offscreen, 0, LODGE_TEXTURE_FORMAT_RGBA16F);
			lodge_gfx_bind_texture_unit_2d(1, system->texture_offscreen, system->sampler_linear_clamp);
			// TODO(TS): sum these in compute shader already
			lodge_gfx_bind_texture_unit_2d(2, hdr->bloom_upsample_texture_levels[1], system->sampler_linear_clamp);
			lodge_gfx_bind_texture_unit_2d(3, hdr->bloom_upsample_texture_levels[2], system->sampler_linear_clamp);
			lodge_gfx_bind_texture_unit_2d(4, hdr->bloom_upsample_texture_levels[3], system->sampler_linear_clamp);
			lodge_gfx_bind_texture_unit_2d(5, hdr->bloom_upsample_texture_levels[4], system->sampler_linear_clamp);
			lodge_gfx_bind_texture_unit_2d(6, hdr->bloom_upsample_texture_levels[5], system->sampler_linear_clamp);
			lodge_gfx_bind_texture_unit_2d(7, hdr->bloom_upsample_texture_levels[6], system->sampler_linear_clamp);
			//lodge_gfx_bind_texture_unit_2d(8, hdr->bloom_upsample_texture_levels[7], system->sampler_linear_clamp);
			lodge_shader_bind_constant_buffer(hdr_resolve_shader, 3, hdr->hdr_resolve_buffer_object);
			lodge_shader_set_constant_vec2(hdr_resolve_shader, strview("resolution"), vec2_make(render_size.width, render_size.height));
			lodge_shader_dispatch_compute(groups_x, groups_y, 1);
			lodge_gfx_annotate_end();
		}
		lodge_gfx_annotate_end();
	}

	// Flip off-screen to screen
	{
		struct lodge_recti src_rect = {
			.x0 = (int32_t)0,
			.y0 = (int32_t)0,
			.x1 = (int32_t)render_size.width,
			.y1 = (int32_t)render_size.height
		};
		struct lodge_recti dst_rect = {
			.x0 = (int32_t)0,
			.y0 = (int32_t)0,
			.x1 = (int32_t)system->window_width,
			.y1 = (int32_t)system->window_height
		};
		lodge_framebuffer_bind(render_params->framebuffer);
		lodge_gfx_set_viewport(0, 0, system->window_width, system->window_height);
		lodge_gfx_set_scissor(0, 0, system->window_width, system->window_height);
		lodge_framebuffer_copy(render_params->framebuffer, system->framebuffer_offscreen, dst_rect, src_rect);
		lodge_framebuffer_unbind();
	}

	lodge_gfx_annotate_end();
	lodge_pipeline_pop();
}

static lodge_system_type_t lodge_scene_render_system_type_register(struct lodge_scene_renderer_plugin *plugin)
{
	ASSERT(!LODGE_SYSTEM_TYPE_SCENE_RENDER);

	if(!LODGE_SYSTEM_TYPE_SCENE_RENDER) {

		lodge_type_t lodge_type_distance_fog = lodge_type_register_property_object(
			strview("distance_fog"),
			sizeof(struct lodge_distance_fog),
			&(struct lodge_properties) {
				.count = 3,
				.elements = {
					{
						.name = strview("density"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_distance_fog, density),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
						.hints = {
							.enable = true,
							.f32 = {
								.min = 0.0f,
								.max = 1.0f,
								.step = 0.001f,
							},
						}
					},
					{
						.name = strview("color"),
						.type = LODGE_TYPE_VEC3_COLOR,
						.offset = offsetof(struct lodge_distance_fog, color),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview("sun_color"),
						.type = LODGE_TYPE_VEC3_COLOR,
						.offset = offsetof(struct lodge_distance_fog, sun_color),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		);

		lodge_type_t lodge_type_static_meshes = lodge_type_register_property_object(
			strview("static_meshes"),
			sizeof(struct lodge_static_meshes),
			&(struct lodge_properties) {
				.count = 1,
				.elements = {
					{
						.name = strview("draw"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_static_meshes, draw),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		);

		lodge_type_t lodge_type_hdr_resolve = lodge_type_register_property_object(
			strview("hdr_resolve"),
			sizeof(struct lodge_hdr_resolve),
			&(struct lodge_properties) {
				.count = 3,
				.elements = {
					{
						.name = strview("gamma"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_hdr_resolve, gamma),
					},
					{
						.name = strview("exposure"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_hdr_resolve, exposure),
					},
					{
						.name = strview("bloom_intensity"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_hdr_resolve, bloom_intensity),
					},
				}
			}
		);

#define lodge_type_entity_ref LODGE_TYPE_U64

		LODGE_SYSTEM_TYPE_SCENE_RENDER = lodge_system_type_register((struct lodge_system_type_desc) {
			.name = strview("scene_renderer"),
			.size = sizeof(struct lodge_scene_render_system),
			.new_inplace = lodge_scene_render_system_new_inplace,
			.free_inplace = NULL,
			.update = lodge_scene_render_system_update,
			.render = lodge_scene_render_system_render,
			.properties = {
				.count = 14,
				.elements = {
					{
						.name = strview("draw_post_process"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_scene_render_system, draw_post_process),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						.name = strview("wireframe"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_scene_render_system, wireframe),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						.name = strview("render_width"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_scene_render_system, render_width),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_render_size,
						.hints = {
							.enable = true,
							.u32 = {
								.min = 0,
								.max = UINT32_MAX,
								.step = 1,
							},
						}
					},
					{
						.name = strview("render_height"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_scene_render_system, render_height),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_render_size,
						.hints = {
							.enable = true,
							.u32 = {
								.min = 0,
								.max = UINT32_MAX,
								.step = 1,
							},
						}
					},
					{
						.name = strview("window_is_render_size"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_scene_render_system, window_is_render_size),
						.on_modified = on_modified_render_size,
					},
					{
						.name = strview("window_width"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_scene_render_system, window_width),
						.flags = LODGE_PROPERTY_FLAG_READ_ONLY | LODGE_PROPERTY_FLAG_TRANSIENT,
					},
					{
						.name = strview("window_height"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_scene_render_system, window_height),
						.flags = LODGE_PROPERTY_FLAG_READ_ONLY | LODGE_PROPERTY_FLAG_TRANSIENT,
					},
					{
						.name = strview("shadow_map_update"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_scene_render_system, shadow_map_update),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						.name = strview("distance_fog"),
						.type = lodge_type_distance_fog,
						.offset = offsetof(struct lodge_scene_render_system, distance_fog),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_distance_fog,
					},
					{
						.name = strview("static_meshes"),
						.type = lodge_type_static_meshes,
						.offset = offsetof(struct lodge_scene_render_system, static_meshes),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						.name = strview("active_camera"),
						.type = lodge_type_entity_ref,
						.offset = offsetof(struct lodge_scene_render_system, active_camera),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						// TODO(TS): bloom as separate object
						.name = strview("bloom_enable"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_scene_render_system, hdr) + offsetof(struct lodge_hdr, bloom_enable),
					},
					{
						.name = strview("bloom_samples_count"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_scene_render_system, hdr) + offsetof(struct lodge_hdr, desired_bloom_samples_count),
						.on_modified = on_modified_bloom_levels,
						.hints = {
							.enable = true,
							.u32 = {
								.min = 0,
								.max = LODGE_ARRAYSIZE(((struct lodge_hdr*)0)->bloom_downsample_texture_levels),
								.step = 1,
							}
						}
					},
					{
						.name = strview("hdr"),
						.type = lodge_type_hdr_resolve,
						.offset = offsetof(struct lodge_scene_render_system, hdr) + offsetof(struct lodge_hdr, hdr_resolve),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_hdr_resolve
					}
				}
			},
			.userdata = plugin,
		});	
	}

	return LODGE_SYSTEM_TYPE_SCENE_RENDER;
}

struct lodge_ret lodge_scene_renderer_plugin_new_inplace(struct lodge_scene_renderer_plugin *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	plugin->shaders = lodge_plugins_depend(plugins, plugin, strview("shaders"));
	ASSERT(plugin->shaders);
	if(!plugin->shaders) {
		return lodge_error("Failed to load `shaders` plugin");
	}

	plugin->textures = lodge_plugins_depend(plugins, plugin, strview("textures"));
	ASSERT(plugin->textures);
	if(!plugin->textures) {
		return lodge_error("Failed to load `textures` plugin");
	}

	plugin->fbx_assets = lodge_plugins_depend(plugins, plugin, strview("fbx"));
	ASSERT(plugin->fbx_assets);
	if(!plugin->fbx_assets) {
		return lodge_error("Failed to load `fbx` plugin");
	}

	plugin->scene_render_system_type = lodge_scene_render_system_type_register(plugin);
	ASSERT(plugin->scene_render_system_type);
	if(!plugin->scene_render_system_type) {
		return lodge_error("Failed to register scene renderer system type");
	}

	struct fbx_types fbx_types = lodge_plugin_fbx_get_types(plugin->fbx_assets);
	struct shader_types shader_types = lodge_plugin_shaders_get_types(plugin->shaders);
	struct texture_types texture_types = lodge_plugin_textures_get_types(plugin->textures);

	plugin->billboard_component_type = lodge_billboard_component_type_register(texture_types.texture_asset_type);
	plugin->billboard_system_type = lodge_billboard_system_type_register(plugin);

	plugin->point_light_component_type = lodge_point_light_component_type_register();
	plugin->directional_light_component_type = lodge_directional_light_component_type_register();
	plugin->static_mesh_component_type = lodge_static_mesh_component_type_register(fbx_types.fbx_asset_type, shader_types.shader_asset_type, texture_types.texture_asset_type);

	return lodge_success();
}

static void lodge_scene_renderer_plugin_free_inplace(struct lodge_scene_renderer_plugin *plugin)
{
	//lodge_plugin_release(plugins, plugin->shaders);
	//lodge_plugin_release(plugins, plugin->textures);
	//lodge_plugin_release(plugins, plugin->fbxes);
}

struct lodge_scene_renderer_types lodge_scene_renderer_plugin_get_types(struct lodge_scene_renderer_plugin *plugin)
{
	return (struct lodge_scene_renderer_types) {
		.scene_render_system = plugin->scene_render_system_type,
		.billboard_system = plugin->billboard_system_type,
		.billboard_component = plugin->billboard_component_type,
		.point_light_component_type = plugin->point_light_component_type,
		.directional_light_component_type = plugin->directional_light_component_type,
		.static_mesh_component_type = plugin->static_mesh_component_type,
	};
}

struct lodge_assets2* lodge_scene_renderer_plugin_get_shaders(struct lodge_scene_renderer_plugin *plugin)
{
	return plugin ? plugin->shaders : NULL;
}

struct lodge_assets2* lodge_scene_renderer_plugin_get_textures(struct lodge_scene_renderer_plugin *plugin)
{
	return plugin ? plugin->textures : NULL;
}

void lodge_scene_add_render_pass_func(lodge_scene_t scene, enum lodge_scene_render_system_pass pass, lodge_scene_render_system_func_t func, void *userdata)
{
	ASSERT(scene);
	ASSERT(LODGE_SYSTEM_TYPE_SCENE_RENDER);
	ASSERT(pass >= 0 && pass < LODGE_SCENE_RENDER_SYSTEM_PASS_MAX);

	struct lodge_scene_render_system *renderer = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_SCENE_RENDER);
	ASSERT(renderer);
	if(!renderer) {
		return;
	}

	struct lodge_render_system_pass *render_pass = &renderer->passes[pass];
	membuf_set(membuf_wrap(render_pass->func_userdatas), render_pass->funcs_count, &userdata, sizeof(void*));
	membuf_append(membuf_wrap(render_pass->funcs), &render_pass->funcs_count, &func, sizeof(lodge_scene_render_system_func_t));
}

void lodge_scene_remove_render_pass_func(lodge_scene_t scene, enum lodge_scene_render_system_pass pass, lodge_scene_render_system_func_t func, void *userdata)
{
	ASSERT(scene);
	ASSERT(LODGE_SYSTEM_TYPE_SCENE_RENDER);
	ASSERT(pass >= 0 && pass < LODGE_SCENE_RENDER_SYSTEM_PASS_MAX);

	struct lodge_scene_render_system *renderer = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_SCENE_RENDER);
	ASSERT(renderer);
	if(!renderer) {
		return;
	}

	struct lodge_render_system_pass *render_pass = &renderer->passes[pass];
	for(size_t i = 0, count = render_pass->funcs_count; i < count; i++) {
		if(render_pass->funcs[i] == func && render_pass->func_userdatas[i] == userdata) {
			membuf_delete(membuf_wrap(render_pass->func_userdatas), &render_pass->funcs_count, i, 1);
			membuf_delete(membuf_wrap(render_pass->funcs), &render_pass->funcs_count, i, 1);
			break;
		}
	}
}

void lodge_scene_set_active_camera(lodge_scene_t scene, lodge_entity_t camera_entity)
{
	ASSERT(scene);
	ASSERT(camera_entity); // TODO(TS): assert entity is part of this scene

	struct lodge_scene_render_system *renderer = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_SCENE_RENDER);
	ASSERT(renderer);
	if(!renderer) {
		return;
	}

	renderer->active_camera = camera_entity;
}

lodge_entity_t lodge_scene_renderer_get_entity_at_screen_pos(struct lodge_scene_render_system *renderer, vec2 screen_pos)
{
	ASSERT_OR(renderer) { return NULL; }

	//
	// TODO(TS): use PBO instead
	//
	lodge_framebuffer_bind(renderer->geometry_buffer.framebuffer);
	
	//
	// Editor color attachment is #2
	//
	vec4 sample = lodge_framebuffer_read_pixel_rgba(2, screen_pos.x, screen_pos.y);
	if(sample.a == 0.0f) {
		return NULL;
	}

	return (lodge_entity_t)((uintptr_t)sample.r);
}

LODGE_PLUGIN_IMPL(lodge_scene_renderer_plugin)
{
	//
	// TODO(TS): if the plugin is reloaded for any reason, the system_type and all instances must be unloaded.
	//
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_scene_renderer_plugin),
		.name = strview("scene_renderer"),
		.new_inplace = lodge_scene_renderer_plugin_new_inplace,
		.free_inplace = lodge_scene_renderer_plugin_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}
