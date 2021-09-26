#include "lodge_foliage_component.h"

#include "math4.h"
#include "dynbuf.h"

#include "lodge_component_type.h"
#include "lodge_static_mesh.h"
#include "lodge_buffer_object.h"
#include "lodge_drawable.h"
#include "lodge_noise.h"

#include <string.h>

struct lodge_foliage_instances
{
	size_t							count;
	size_t							capacity;
	vec4							*elements; // x,y,z,uniform_scale
};

struct lodge_foliage_lod
{
	struct lodge_foliage_instances	instances;
	lodge_asset_t					static_mesh;
	lodge_buffer_object_t			buffer_object;
	lodge_drawable_t				drawable;
};

struct lodge_foliage_component
{
	float							threshold;
	float							axis_divisor;
	uint32_t						instances_count;
	struct lodge_foliage_instances	instances;

	uint32_t						lods_count;
	struct lodge_foliage_lod		lods[LODGE_FOLIAGE_LODS_MAX];
};

//
// TODO(TS): Support parametric placement via custom Node System
//

lodge_component_type_t LODGE_COMPONENT_TYPE_FOLIAGE = NULL;

static void lodge_foliage_lod_free_inplace(struct lodge_foliage_lod *lod)
{
	lodge_drawable_reset(lod->drawable);
	lodge_buffer_object_reset(lod->buffer_object);
	dynbuf_free_inplace(dynbuf_wrap_stack(lod->instances));
}

static void lodge_foliage_lod_set_static_mesh(struct lodge_foliage_lod *lod, lodge_asset_t static_mesh)
{
#if 0
	lod->static_mesh = static_mesh;

	if(lod->static_mesh && lod->buffer_object) {
		struct lodge_drawable_desc drawble_desc = lodge_drawable_desc_make_from_static_mesh(lod->static_mesh);

		drawble_desc.attribs[drawble_desc.attribs_count++] = (struct lodge_drawable_attrib) {
			.name = strview_static("instances"),
			.buffer_object = lod->buffer_object,
			.float_count = 4,
			.offset = 0,
			.stride = sizeof(vec4),
			.instanced = 1
		};

		if(lod->drawable) {
			lodge_drawable_reset(lod->drawable);
		}

		lod->drawable = lodge_drawable_make(drawble_desc);
	}
#endif
}

static void lodge_foliage_lod_set_instances_max(struct lodge_foliage_lod *lod, uint32_t instances_max)
{
	dynbuf_free_inplace(dynbuf_wrap_stack(lod->instances));
	dynbuf_new_inplace(dynbuf_wrap_stack(lod->instances), instances_max);

	if(lod->buffer_object) {
		lodge_buffer_object_reset(lod->buffer_object);
	}

	lod->buffer_object = lodge_buffer_object_make_dynamic(sizeof(vec4) * instances_max);

	lodge_foliage_lod_set_static_mesh(lod, lod->static_mesh);
}

static void lodge_foliage_lod_new_inplace(struct lodge_foliage_lod *lod, uint32_t instances_max)
{
	memset(lod, 0, sizeof(struct lodge_foliage_lod));
	lodge_foliage_lod_set_instances_max(lod, instances_max);
	lodge_foliage_lod_set_static_mesh(lod, NULL);
}

void lodge_foliage_component_new_inplace(struct lodge_foliage_component *foliage)
{
	foliage->axis_divisor = 100.0f;
	foliage->threshold = 1.0f;
	foliage->lods_count = 0;

	foliage->instances_count = 64;
	dynbuf_new_inplace(dynbuf_wrap_stack(foliage->instances), foliage->instances_count);

	for(int lod_idx = 0; lod_idx < LODGE_FOLIAGE_LODS_MAX; lod_idx++) {
		lodge_foliage_lod_new_inplace(&foliage->lods[lod_idx], foliage->instances_count);
	}
}

void lodge_foliage_component_free_inplace(struct lodge_foliage_component *foliage)
{
	for(int lod_idx = 0; lod_idx < LODGE_FOLIAGE_LODS_MAX; lod_idx++) {
		lodge_foliage_lod_free_inplace(&foliage->lods[lod_idx]);
	}
	dynbuf_free_inplace(dynbuf_wrap_stack(foliage->instances));
}

static void on_modified_instances_count(struct lodge_property *property, struct lodge_foliage_component *foliage)
{
	dynbuf_t instances = dynbuf_wrap_stack(foliage->instances);

	dynbuf_clear(instances);
#if 0
	for(uint32_t i = 0, count = foliage->instances_count; i < count; i++) {
		vec4 *element = dynbuf_append_no_init(instances);

		element->x = randr(0.0f, 1.0f);
		element->y = randr(0.0f, 1.0f);
		element->z = randr(0.0f, 0.0f);
		element->w = randr(1.0f, 10.0f);
	}
#else
	//
	// TODO(TS): do this on-demand in render call and populate only close to camera
	//
	foliage->instances_count = 0;
	const float cell_size = 0.01f;
	const float step_size = 1.0f / foliage->axis_divisor;
	for(float y = 0.0f; y <= 1.0f; y+=step_size) {
		for(float x = 0.0f; x <= 1.0f; x+=step_size) {
			const float noise = lodge_noise_simplex_2d(x, y);
			if(noise >= foliage->threshold) {
				vec4 *element = dynbuf_append_no_init(instances);

				element->x = x + clamp(noise, -cell_size, cell_size);
				element->y = y + clamp(noise, -cell_size, cell_size);
				element->z = 0.0f;
				element->w = 1.0f + noise * 10;

				foliage->instances_count++;
			}
		}
	}
#endif

	for(uint32_t i = 0; i < foliage->lods_count; i++) {
		lodge_foliage_lod_set_instances_max(&foliage->lods[i], foliage->instances_count);
	}
}

lodge_component_type_t lodge_foliage_component_type_register()
{
	if(!LODGE_COMPONENT_TYPE_FOLIAGE) {
		LODGE_COMPONENT_TYPE_FOLIAGE = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("foliage"),
			.description = strview_static("Adds parametric foliage to terrain."),
			.new_inplace = lodge_foliage_component_new_inplace,
			.free_inplace = lodge_foliage_component_free_inplace,
			.size = sizeof(struct lodge_foliage_component),
			.properties = {
				.count = 3,
				.elements = {
					{
						.name = strview_static("instances_count"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_foliage_component, instances_count),
						.flags = LODGE_PROPERTY_FLAG_READ_ONLY,
						.on_modified = NULL,
					},
					{
						.name = strview_static("threshold"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_foliage_component, threshold),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_instances_count,
					},
					{
						.name = strview_static("axis_divisor"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_foliage_component, axis_divisor),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_instances_count,
					},
				}
			}
		});
	}
	return LODGE_COMPONENT_TYPE_FOLIAGE;
}

// FIXME(TS): foliage system?

#include "lodge_gfx.h"
#include "lodge_shader.h"
#include "lodge_scene.h"
#include "lodge_plugin_scene_renderer.h"
#include "lodge_terrain_system.h"
#include "lodge_terrain_component.h"
#include "lodge_transform_component.h"

void lodge_foliage_system_render(struct lodge_foliage_component *foliage, const struct lodge_scene_render_pass_params *pass_params, lodge_shader_t shader, lodge_sampler_t heightfield_sampler, struct lodge_terrain_component *terrain, lodge_entity_t owner, vec3 terrain_scale)
{
#if 0
	ASSERT_OR(shader) {
		return;
	}

	ASSERT_OR(foliage) {
		return;
	}

	lodge_gfx_annotate_begin(strview_static("foliage"));
	{
		lodge_gfx_bind_shader(shader);

		lodge_shader_set_constant_float(shader, strview_static("time"), pass_params->time);
		lodge_shader_set_constant_vec3(shader, strview_static("terrain_scale"), terrain_scale);
		lodge_gfx_bind_texture_unit_2d(0, terrain->heightmap, heightfield_sampler);

		//
		// FIXME(TS): debugging, puts all instances into LOD 0
		//
		if(foliage->lods_count > 0) {
			dynbuf_clear(dynbuf_wrap_stack(foliage->lods[0].instances));
			dynbuf_append_range(dynbuf_wrap_stack(foliage->lods[0].instances), foliage->instances.elements, sizeof(vec4), foliage->instances.count);
		}

		for(size_t lod_idx = 0; lod_idx < foliage->lods_count; lod_idx++) {
			struct lodge_foliage_lod *lod = &foliage->lods[lod_idx];

			if(lod->instances.count > 0) {
				if(lod->drawable && lod->static_mesh) {
					lodge_buffer_object_set(
						lod->buffer_object,
						0,
						lod->instances.elements,
						lod->instances.count * sizeof(vec4)
					);

					lodge_drawable_render_indexed_instanced(
						lod->drawable,
						lod->static_mesh->indices_count,
						lod->instances.count
					);
				}
			}
		}
	}
	lodge_gfx_annotate_end();
#endif
}

void lodge_foliage_set_lods_desc(struct lodge_foliage_component *foliage, struct lodge_foliage_lods_desc *lods_desc)
{
#if 0
	foliage->lods_count = lods_desc->count;

	for(uint32_t i = 0; i < lods_desc->count; i++) {
		lodge_foliage_lod_set_static_mesh(&foliage->lods[i], lods_desc->elements[i]);
	}
#endif
}