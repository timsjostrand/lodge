#include "frustum.h"

#include <float.h>

#if 0

struct frustum_planes frustum_planes_make_from_projection(const mat4 mat)
{
	struct frustum_planes tmp;

	for (int i = 4; i--; ) tmp.planes[FRUSTUM_PLANE_LEFT].v[i]		= mat.rows[i].v[3] + mat.rows[i].v[0];
	for (int i = 4; i--; ) tmp.planes[FRUSTUM_PLANE_RIGHT].v[i]		= mat.rows[i].v[3] - mat.rows[i].v[0]; 
	for (int i = 4; i--; ) tmp.planes[FRUSTUM_PLANE_TOP].v[i]		= mat.rows[i].v[3] + mat.rows[i].v[1];
	for (int i = 4; i--; ) tmp.planes[FRUSTUM_PLANE_BOTTOM].v[i]	= mat.rows[i].v[3] - mat.rows[i].v[1];
	for (int i = 4; i--; ) tmp.planes[FRUSTUM_PLANE_NEAR].v[i]		= mat.rows[i].v[3] + mat.rows[i].v[2];
	for (int i = 4; i--; ) tmp.planes[FRUSTUM_PLANE_FAR].v[i]		= mat.rows[i].v[3] - mat.rows[i].v[2];
	
	return tmp;
}

bool frustum_planes_is_visible_sphere(struct frustum_planes *frustum, struct sphere sphere)
{
	for(int i = 0; i < 6; i++)
	{
		float dist = vec3_dot(sphere.pos, vec3_make(xyz_of(frustum->planes[i]))) + frustum->planes[i].w + sphere.r;
		if(dist < 0) return false;
	}
	return true;
}

#endif

struct frustum_corners frustum_corners_make_ndc()
{
	struct frustum_corners corners = { 0 };

	for(uint32_t z = 0; z < 2; z++) {
		for(uint32_t y = 0; y < 2; y++) {
			for(uint32_t x = 0; x < 2; x++) {
				corners.vertices[x][y][z] = vec3_make(
					(x ? -1.0f : 1.0f),
					(y ? -1.0f : 1.0f),
					(z ?  -1.0f : 1.0f) // 0.0f .. 1.0f for D3D
				);
			}
		}
	}

	return corners;
}

struct frustum_corners frustum_corners_make(const mat4 inv_view_proj)
{
	struct frustum_corners corners = frustum_corners_make_ndc();
	return frustum_corners_transform(&corners, inv_view_proj);
}

struct frustum_corners frustum_corners_transform(const struct frustum_corners *corners, const mat4 transform)
{
	struct frustum_corners ret = { 0.0f };

	for(uint32_t z = 0; z < 2; z++) {
		for(uint32_t y = 0; y < 2; y++) {
			for(uint32_t x = 0; x < 2; x++) {
				const vec4 tmp = mat4_mult_vec4(transform, vec4_make_from_vec3(corners->vertices[x][y][z], 1.0f));

				ret.vertices[x][y][z] = (vec3) {
					.x = tmp.x / tmp.w,
					.y = tmp.y / tmp.w,
					.z = tmp.z / tmp.w
				};
			}
		}
	}

	return ret;
}


vec3 frustum_corners_calc_center(const struct frustum_corners *corners)
{
	vec3 center = { 0 };

	for(uint32_t z = 0; z < 2; z++) {
		for(uint32_t y = 0; y < 2; y++) {
			for(uint32_t x = 0; x < 2; x++) {
				center = vec3_add(center, corners->vertices[x][y][z]);
			}
		}
	}

	return vec3_div_scalar(center, 8);
}

struct aabb frustum_corners_calc_bounds(const struct frustum_corners *corners)
{
	struct aabb bounds = {
		.min = { FLT_MAX },
		.max = { -FLT_MAX }
	};

	for(uint32_t z = 0; z < 2; z++) {
		for(uint32_t y = 0; y < 2; y++) {
			for(uint32_t x = 0; x < 2; x++) {
				const vec3 *vertex = &corners->vertices[x][y][z];
				bounds.min = vec3_min(*vertex, bounds.min);
				bounds.max = vec3_max(*vertex, bounds.max);
			}
		}
	}

	return bounds;
}
