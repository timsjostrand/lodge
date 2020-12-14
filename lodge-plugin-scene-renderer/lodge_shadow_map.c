#include "lodge_shadow_map.h"

#include "frustum.h"

#include "lodge_framebuffer.h"
#include "lodge_texture.h"
#include "lodge_shader.h"
#include "lodge_buffer_object.h"

void lodge_shadow_map_new_inplace(struct lodge_shadow_map *shadow_map, uint32_t width, uint32_t height, float z_near, float z_far)
{
	shadow_map->width = width;
	shadow_map->height = height;
	shadow_map->near_factor = 6.0f;
	shadow_map->far_factor = 6.0f;

#if 0
	for(uint32_t i = 0; i < LODGE_SHADOW_MAP_CASCADE_SPLITS; ++i) {
		const float p = (i + 1) / (float)LODGE_SHADOW_MAP_CASCADE_SPLITS;
		const float log = z_near * pow(ratio, p);
		const float uniform = z_near + range * p;
		const float d = lambda * (log - uniform) + uniform;
		shadow_map->cascade_splits[i] = (d - nearClip) / clipRange;
	}
#else
	shadow_map->buffer.cascade_splits.v[0] = 0.05f;
	shadow_map->buffer.cascade_splits.v[1] = 0.2f;
	shadow_map->buffer.cascade_splits.v[2] = 1.0f;

	shadow_map->buffer.cascade_splits_worldspace.v[0] = shadow_map->buffer.cascade_splits.v[0] * (z_far - z_near);
	shadow_map->buffer.cascade_splits_worldspace.v[1] = shadow_map->buffer.cascade_splits.v[1] * (z_far - z_near);
	shadow_map->buffer.cascade_splits_worldspace.v[2] = shadow_map->buffer.cascade_splits.v[2] * (z_far - z_near);

	shadow_map->buffer_object = lodge_buffer_object_make_dynamic(sizeof(struct lodge_shadow_map_buffer));
#endif

	shadow_map->depth_textures_array = lodge_texture_2d_array_make_depth(width, height, LODGE_SHADOW_CASCADE_SPLITS_COUNT);
	shadow_map->framebuffer = lodge_framebuffer_make((struct lodge_framebuffer_desc) {
		.colors_count = 0,
		.stencil = NULL,
		.depth = shadow_map->depth_textures_array
	});
}

vec3 vec3_transform(const vec3 p, const mat4 *m)
{
	vec4 tmp = mat4_mult_vec4(*m, vec4_make(xyz_of(p), 1.0f));
	return vec3_make(xyz_of(tmp));
}

static vec3 lodge_shadow_map_snap_to_texel(vec3 point_world_space, vec3 light_dir, float texels_per_unit)
{
	const mat4 scaling = mat4_scaling(texels_per_unit, texels_per_unit, texels_per_unit);
	const mat4 look_unscaled = mat4_look(vec3_zero(), vec3_negate(light_dir), vec3_make(0.0f, 0.0f, 1.0f));
	const mat4 look = mat4_mult(scaling, look_unscaled);
	const mat4 look_inv = mat4_inverse(look, NULL);

	vec3 point_light_space = vec3_transform(point_world_space, &look);
	point_light_space.x = floorf(point_light_space.x);
	point_light_space.y = floorf(point_light_space.y);
	point_light_space.z = floorf(point_light_space.z);

	return vec3_transform(point_light_space, &look_inv);
}

//
// Stable Shadow Mapping from http://alextardif.com/shadowmapping.html
//
void lodge_shadow_map_update(struct lodge_shadow_map *shadow_map, float dt, const mat4 inv_view_proj, vec3 light_dir, struct lodge_shadow_map_debug debug_out[LODGE_SHADOW_CASCADE_SPLITS_COUNT])
{
	//
	// Fit the light projection inside the camera frustum
	//
	for(uint32_t cascade_index = 0; cascade_index < LODGE_SHADOW_CASCADE_SPLITS_COUNT; cascade_index++) {
		const float cascade_dist_min = cascade_index > 0 ? shadow_map->buffer.cascade_splits.v[cascade_index - 1] : 0.0f;
		const float cascade_dist_max = shadow_map->buffer.cascade_splits.v[cascade_index];
		const struct frustum_corners frustum = frustum_corners_make_slice(inv_view_proj, cascade_dist_min, cascade_dist_max);

		static float last_radius = 0.0f;
		float radius = 0.0f;
		{
			const struct aabb frustum_bounds = frustum_corners_calc_bounds(&frustum);
			radius = vec3_distance(frustum_bounds.min, frustum_bounds.max) / 2.0f;
			
			// FIXME(TS): radius differs between frames for unknown reasons, this is a hack
			{
				radius = (int)radius / (int)100;
				radius *= 100.0f;
			}

			if(cascade_index == 0) {
				if(last_radius > 0.0f)
					//ASSERT(radius == last_radius);
				last_radius = radius;
			}
		}

		const float texels_per_unit = shadow_map->width / (radius * 2.0f);
		const vec3 frustum_center = lodge_shadow_map_snap_to_texel(frustum_corners_calc_center(&frustum), light_dir, texels_per_unit);

		const vec3 light_pos = vec3_sub(frustum_center, vec3_mult_scalar(light_dir, radius * 2.0f));
		const mat4 view_lightspace = mat4_look(light_pos, vec3_negate(light_dir), vec3_make(0.0f, 0.0f, 1.0f));

		const mat4 projection_lightspace = mat4_ortho(
			-radius, radius,
			-radius, radius,
			-radius * shadow_map->near_factor, radius * shadow_map->far_factor
		);

		shadow_map->buffer.views[cascade_index] = view_lightspace;
		shadow_map->buffer.projections[cascade_index] = projection_lightspace;

#if 0
		if(debug_out) {
			debug_out[cascade_index] = (struct lodge_shadow_map_debug) {
				.light_pos = light_pos,
				.frustum_center = frustum_center,
				.p_max = vec3_zero(), //frustum_bounds_lightspace.max,
				.p_min = vec3_zero(), //frustum_bounds_lightspace.min,
				.frustum = frustum,
				.frustum_bounds = (struct aabb) { 0 }, //frustum_bounds,
				.texture = shadow_map->depth_textures_array,
			};
		}
#endif
	}

	lodge_buffer_object_set(shadow_map->buffer_object, 0, &shadow_map->buffer, sizeof(struct lodge_shadow_map_buffer));
}