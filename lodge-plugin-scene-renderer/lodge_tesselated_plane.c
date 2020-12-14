#include "lodge_tesselated_plane.h"

#include "math4.h"
#include "dynbuf.h"
#include "coordinate_systems.h"
#include "gruvbox.h"

#include "lodge_quadtree.h"
#include "lodge_assert.h"
#include "lodge_debug_draw.h"

#include "lodge_plugin_scene_renderer.h" // for camera_params

#include <stdint.h>
#include <stdbool.h>

struct lodge_tesselated_plane
{
	struct lodge_tesselated_plane_chunk	*elements;
	size_t								count;
	size_t								capacity;
};

struct lodge_tesselated_plane_is_leaf_params
{
	struct lodge_tesselated_plane		*plane;
	vec3								pos;
	vec3								scale;
	const struct lodge_camera_params	*camera;
	uint32_t							level_max;
	float								lod_switch_distance;
	struct lodge_debug_draw				*debug_draw;
};

void lodge_tesselated_plane_new_inplace(struct lodge_tesselated_plane *plane)
{
	plane->count = 0;
	plane->capacity = 32;
	plane->elements = NULL;
}

void lodge_tesselated_plane_free_inplace(struct lodge_tesselated_plane *plane)
{
	if(plane->elements) {
		free(plane->elements);
	}
}

size_t lodge_tesselated_plane_sizeof()
{
	return sizeof(struct lodge_tesselated_plane);
}

static bool lodge_tesselated_plane_is_leaf_impl(const uint32_t level, const vec2 center, const vec2 size, struct lodge_tesselated_plane_is_leaf_params *userdata)
{
	if(level == 0) {
		return false;
	}

	if(level == userdata->level_max) {
		return true;
	}

#if 0
	// vec3_negate(camera->pos)
	if(camera->pos.x >= (center.x - size.x)
		&& camera->pos.x <= (center.x + size.x)
		&& camera->pos.y >= (center.y - size.y)
		&& camera->pos.y <= (center.y + size.y)) {
		return false;
	}
#else

#if 0
	const vec3 center_v3 = vec3_make(center.x, center.y, 0.0f);
	const vec3 size_half = vec3_make(size.x/2.0f, size.y/2.0f, userdata->scale.z/2.0f); // TODO(TS): account for scale
	const vec3 p0 = vec3_add(center_v3, size_half);
	const vec3 p1 = vec3_sub(center_v3, size_half);

	// FIXME(TS): ndc vs clip?

	const vec3 p0_ndc_space = vec3_world_to_ndc_space(p0, &userdata->camera->view_projection);
	const vec3 p1_ndc_space = vec3_world_to_ndc_space(p1, &userdata->camera->view_projection);

	const bool in_clip = vec3_is_in_clip(p0_ndc_space) && vec3_is_in_clip(p1_ndc_space);

	// FIXME(TS): Not true, only for testing
	//if(!in_clip) {
	//	return true;
	//}

	//if(!vec3_is_in_clip(p0_clip_space) || vec3_is_in_clip(p1_clip_space)) {
	{
		const float clip_dist = vec3_distance(p0_ndc_space, p1_ndc_space);

		if(clip_dist >= userdata->lod_switch_distance) {
			return false;
		}
	}
#else

	// Neat approximation of projected sphere:
	//
	//		https://stackoverflow.com/questions/3717226/radius-of-projected-sphere
	//

	//
	// FIXME(TS): take min/max height into consideration
	//
	const float fov_cot = 1.0f / tanf(userdata->camera->fov_y / 2.0f);
	const float z = vec3_distance(userdata->camera->pos, vec3_make(center.x, center.y, 0.0f));
	const float radius_worldspace = vec2_length(size);
	const float radius_clipspace_approx = radius_worldspace * fov_cot / z;

	if(radius_clipspace_approx*2.0f >= userdata->lod_switch_distance) {
		if(userdata->debug_draw) {
			lodge_debug_draw_sphere(
				userdata->debug_draw,
				(struct sphere) {
					.pos = vec3_make(center.x, center.y, 0.0f),
					.r = radius_worldspace
				},
				vec4_make(xyz_of(GRUVBOX_BRIGHT_YELLOW), 0.1f),
				0.0f
			);
		}
		return false;
	}

#endif

#endif

	return true;
}

static bool lodge_tesselated_plane_is_leaf(const uint32_t level, const vec2 center, const vec2 size, struct lodge_tesselated_plane_is_leaf_params *userdata)
{
	ASSERT(userdata);
	
	const bool is_leaf = lodge_tesselated_plane_is_leaf_impl(
		level,
		center,
		size,
		userdata
	);

	if(is_leaf) {
		struct lodge_tesselated_plane_chunk *chunk = dynbuf_append_no_init(dynbuf_wrap(userdata->plane));

		*chunk = (struct lodge_tesselated_plane_chunk) {
			.lod = userdata->level_max - level,
			.center = center,
			.size = size,
		};
	}

	//
	// If not leaf: split again.
	//
	return is_leaf;
}

void lodge_tesselated_plane_update(struct lodge_tesselated_plane *plane, vec3 pos, vec3 scale, const struct lodge_camera_params *camera, uint32_t level_max, float lod_switch_distance, struct lodge_debug_draw *debug_draw)
{
	struct lodge_tesselated_plane_is_leaf_params params = {
		.plane = plane,
		.pos = pos,
		.scale = scale,
		.camera = camera,
		.level_max = level_max,
		.lod_switch_distance = lod_switch_distance,
		.debug_draw = debug_draw,
	};

	plane->count = 0;
	lodge_quadtree_make(vec2_make(pos.x, pos.y), vec2_make(scale.x, scale.y), &lodge_tesselated_plane_is_leaf, &params);
}

struct lodge_tesselated_plane_chunk* lodge_tesselated_plane_chunks_begin(struct lodge_tesselated_plane *plane)
{
	if(!plane || !plane->count) {
		return NULL;
	}
	return &plane->elements[0];
}

struct lodge_tesselated_plane_chunk* lodge_tesselated_plane_chunks_end(struct lodge_tesselated_plane *plane)
{
	if(!plane || !plane->count) {
		return NULL;
	}
	return &plane->elements[plane->count];
}