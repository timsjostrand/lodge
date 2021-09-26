#include "lodge_billboard_system.h"

#include "dynbuf.h"

#include "lodge_billboard_component.h"
#include "lodge_system_type.h"
#include "lodge_scene.h"
#include "lodge_plugin_scene_renderer.h"
#include "lodge_transform_component.h"
#include "lodge_component_type.h"
#include "lodge_assets2.h"

#include "lodge_gfx.h"
#include "lodge_shader.h"
#include "lodge_texture.h"
#include "lodge_sampler.h"
#include "lodge_drawable.h"
#include "lodge_buffer_object.h"

#include <stdbool.h>

#define LODGE_GEN_VAR(name) name ## __LINE__
#define LODGE_WRAP_SCOPE(begin, end) for( \
	int LODGE_GEN_VAR(_i_) = (begin, 0); \
	!LODGE_GEN_VAR(_i_); \
	(LODGE_GEN_VAR(_i_) += 1, end)) \

#define lodge_gfx_annotate_scope(message) LODGE_WRAP_SCOPE(lodge_gfx_annotate_begin(message), lodge_gfx_annotate_end(message))

struct billboard_render_data
{
	lodge_texture_t					texture;
	vec3							pos;
	vec2							billboard_size;
	mat4							model;
};

struct billboard_render_datas
{
	size_t							count;
	size_t							capacity;
	struct billboard_render_data	*elements;
};

struct lodge_billboard_system
{
	bool							draw;

	lodge_component_type_t			billboard_component_type;

	lodge_asset_t					shader_asset;
	lodge_shader_t					shader;

	lodge_buffer_object_t			pos_uv_buffer_object;
	lodge_drawable_t				drawable;
	lodge_sampler_t					sampler;

	struct billboard_render_datas	render_datas;
};

static void lodge_billboard_system_render(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, struct lodge_billboard_system *system)
{
	if(!system->draw) {
		return;
	}

	if(!system->shader) {
		return;
	}

	ASSERT(system->billboard_component_type);
	ASSERT(pass_params->pass == LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED);

	lodge_gfx_annotate_scope(strview("billboard"))
	{
		lodge_gfx_bind_shader(system->shader);

		for(size_t i = 0, count = system->render_datas.count; i < count; i++) {
			struct billboard_render_data *render_data = &system->render_datas.elements[i];

			lodge_gfx_bind_texture_unit_2d(2, render_data->texture, system->sampler);

			lodge_shader_set_constant_vec3(system->shader, strview("billboard_pos"), render_data->pos);
			lodge_shader_set_constant_vec2(system->shader, strview("billboard_size"), render_data->billboard_size);
			lodge_shader_set_constant_mat4(system->shader, strview("model"), render_data->model);

			lodge_drawable_render_triangles_instanced(system->drawable, 0, 6, 1);
		}
	}
}

static void lodge_billboard_system_new_inplace(struct lodge_billboard_system *system, lodge_scene_t scene)
{
	system->draw = true;

	{
		const float x = 0.0f;
		const float y = 0.0f;

		const float w = 1.0f / 2.0f;
		const float h = 1.0f / 2.0f;

		const float vertex_data[] = {
			/* Top-left */
			x - w,		// x
			y + h,		// y
			0.0f,		// z
			0.0f,		// u
			0.0f,		// v
			/* Bottom-Left */
			x - w,		// x
			y - h,		// y
			0.0f,		// z
			0.0f,		// u
			1.0f,		// v
			/* Top-right */
			x + w,		// x
			y + h,		// y
			0.0f,		// z
			1.0f,		// u
			0.0f,		// v
			/* Top-right */
			x + w,		// x
			y + h,		// y
			0.0f,		// z
			1.0f,		// u
			0.0f,		// v
			/* Bottom-left */
			x - w,		// x
			y - h,		// y
			0.0f,		// z
			0.0f,		// u
			1.0f,		// v
			/* Bottom-right */
			x + w,		// x
			y - h,		// y
			0.0f,		// z
			1.0f,		// u
			1.0f,		// v
		};

		system->pos_uv_buffer_object = lodge_buffer_object_make_static(vertex_data, sizeof(vertex_data));
	}

	system->sampler = lodge_sampler_make((struct lodge_sampler_desc) {
		.min_filter = MAG_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE
	});

	system->drawable = lodge_drawable_make((struct lodge_drawable_desc) {
		.attribs_count = 2,
		.attribs = {
			[0] = {
				.name = strview("pos"),
				.buffer_object = system->pos_uv_buffer_object,
				.offset = 0,
				.float_count = 3,
				.stride = 5 * sizeof(float),
				.instanced = 0
			},
			[1] = {
				.name = strview("uv"),
				.buffer_object = system->pos_uv_buffer_object,
				.offset = 3 * sizeof(float),
				.float_count = 2,
				.stride = 5 * sizeof(float),
				.instanced = 0
			},
		}
	});

	system->billboard_component_type = lodge_component_types_find(strview("billboard"));
	ASSERT(system->billboard_component_type);

	dynbuf_new_inplace(dynbuf(system->render_datas), 128);

	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED, &lodge_billboard_system_render, system);
}

static void lodge_billboard_system_free_inplace(struct lodge_billboard_system *system, lodge_scene_t scene)
{
	dynbuf_free_inplace(dynbuf(system->render_datas));

	lodge_scene_remove_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_DEFERRED, lodge_billboard_system_render, system);

	lodge_buffer_object_reset(system->pos_uv_buffer_object);
	lodge_sampler_reset(&system->sampler);
	lodge_drawable_reset(system->drawable);
}

static void lodge_billboard_system_update(struct lodge_billboard_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt)
{
	struct lodge_scene_renderer_plugin *plugin = lodge_system_type_get_plugin(type);
	ASSERT_OR(plugin) {
		return;
	}

	// FIXME(TS): hardcoded file paths

	if(!system->shader) {
		struct lodge_assets2 *shaders = lodge_scene_renderer_plugin_get_shaders(plugin);
		ASSERT_OR(shaders) {
			return;
		}
		system->shader_asset = lodge_assets2_register(shaders, strview("billboard"));
		system->shader = lodge_assets2_get(shaders, system->shader_asset);
	}

	//
	// TODO(TS):
	// 1) update vertex buffer with instances
	// 2) update texture atlas with sprites -- but probably start with multiple draw calls (1 per texture) and check perf
	//

	struct lodge_assets2 *textures = lodge_scene_renderer_plugin_get_textures(plugin);

	{
		dynbuf_clear(dynbuf(system->render_datas));
	}

	lodge_scene_components_foreach(scene, struct lodge_billboard_component*, billboard, system->billboard_component_type) {
		lodge_texture_t *texture = lodge_assets2_get(textures, billboard->texture_asset);
		if(!texture) {
			continue;
		}

		lodge_entity_t owner = lodge_scene_get_component_entity(scene, system->billboard_component_type, billboard);

		//const vec3 scale = lodge_get_scale(scene, owner);
		const vec3 pos = lodge_get_position(scene, owner);
		const mat4 translation = mat4_translation(xyz_of(pos));
		const mat4 scale = mat4_scaling(billboard->size.x, billboard->size.y, 0.0f);
		const mat4 trs = mat4_mult(translation, scale);

		struct billboard_render_data *render_data = dynbuf_append_no_init(dynbuf(system->render_datas));
		render_data->texture = *texture;
		render_data->pos = pos;
		render_data->billboard_size = billboard->size;
		render_data->model = trs;
	}
}

lodge_system_type_t lodge_billboard_system_type_register(struct lodge_scene_renderer_plugin *plugin)
{
	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview("billboard_system"),
		.size = sizeof(struct lodge_billboard_system),
		.new_inplace = lodge_billboard_system_new_inplace,
		.free_inplace = lodge_billboard_system_free_inplace,
		.update = lodge_billboard_system_update,
		.plugin = plugin,
		.properties = {
			.count = 1,
			.elements = {
				{
					.name = strview("draw"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_billboard_system, draw),
					.flags = LODGE_PROPERTY_FLAG_NONE,
				},
			}
		}
	});
}
