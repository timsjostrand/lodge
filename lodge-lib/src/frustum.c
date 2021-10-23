#include "frustum.h"

#include <float.h>

struct frustum_planes frustum_planes_make(const mat4 view_proj)
{
	struct frustum_planes tmp;

	for(int i = 4; i--;) tmp.planes[FRUSTUM_PLANE_LEFT].v[i]	= view_proj.rows[i].v[3] + view_proj.rows[i].v[0];
	for(int i = 4; i--;) tmp.planes[FRUSTUM_PLANE_RIGHT].v[i]	= view_proj.rows[i].v[3] - view_proj.rows[i].v[0]; 
	for(int i = 4; i--;) tmp.planes[FRUSTUM_PLANE_TOP].v[i]		= view_proj.rows[i].v[3] + view_proj.rows[i].v[1];
	for(int i = 4; i--;) tmp.planes[FRUSTUM_PLANE_BOTTOM].v[i]	= view_proj.rows[i].v[3] - view_proj.rows[i].v[1];
	for(int i = 4; i--;) tmp.planes[FRUSTUM_PLANE_NEAR].v[i]	= view_proj.rows[i].v[3] + view_proj.rows[i].v[2];
	for(int i = 4; i--;) tmp.planes[FRUSTUM_PLANE_FAR].v[i]		= view_proj.rows[i].v[3] - view_proj.rows[i].v[2];
	
	return tmp;
}

//
// https://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
//
bool frustum_planes_vs_aabb(struct frustum_planes *frustum, struct aabb aabb)
{
	//
    // Check box outside/inside of frustum
	//
    for(int i=0; i < FRUSTUM_PLANE_MAX; i++) {
        int out = 0;
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        out += ((vec4_dot( frustum->planes[i], vec4_make(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f) ) < 0.0 ) ? 1 : 0);
        if( out==8 ) return false;
    }

#if 0
	//
    // Check frustum outside/inside box
	//
    int out;
    out=0; for(int i=0; i<8; i++) out += ((frustum->vertices[i].x > aabb.max.x) ? 1 : 0); if(out==8) return false;
    out=0; for(int i=0; i<8; i++) out += ((frustum->vertices[i].x < aabb.min.x) ? 1 : 0); if(out==8) return false;
    out=0; for(int i=0; i<8; i++) out += ((frustum->vertices[i].y > aabb.max.y) ? 1 : 0); if(out==8) return false;
    out=0; for(int i=0; i<8; i++) out += ((frustum->vertices[i].y < aabb.min.y) ? 1 : 0); if(out==8) return false;
    out=0; for(int i=0; i<8; i++) out += ((frustum->vertices[i].z > aabb.max.z) ? 1 : 0); if(out==8) return false;
    out=0; for(int i=0; i<8; i++) out += ((frustum->vertices[i].z < aabb.min.z) ? 1 : 0); if(out==8) return false;
#endif

    return true;
}

bool frustum_planes_vs_sphere(struct frustum_planes *frustum, struct sphere *sphere)
{
	for(int i = 0; i < 6; i++) {
		float dist = vec3_dot(sphere->pos, vec3_make(xyz_of(frustum->planes[i]))) + frustum->planes[i].w + sphere->r;
		if(dist < 0) return false;
	}
	return true;
}

struct frustum_corners frustum_corners_make_ndc()
{
	return (struct frustum_corners) {
		.vertices = {
			vec3_make(-1.0f,  1.0f, -1.0f),
			vec3_make( 1.0f,  1.0f, -1.0f),
			vec3_make( 1.0f, -1.0f, -1.0f),
			vec3_make(-1.0f, -1.0f, -1.0f),
			vec3_make(-1.0f,  1.0f,  1.0f),
			vec3_make( 1.0f,  1.0f,  1.0f),
			vec3_make( 1.0f, -1.0f,  1.0f),
			vec3_make(-1.0f, -1.0f,  1.0f)
		}
	};
}

struct frustum_corners frustum_corners_make(const mat4 inv_view_proj)
{
	struct frustum_corners corners = frustum_corners_make_ndc();
	return frustum_corners_transform(&corners, inv_view_proj);
}

struct frustum_corners frustum_corners_make_slice(const mat4 inv_view_proj, float dist_min_01, float dist_max_01)
{
	struct frustum_corners corners = frustum_corners_make(inv_view_proj);

	for(uint32_t i = 0; i < 4; ++i) {
		const vec3 corner_ray = vec3_sub(corners.vertices[i + 4], corners.vertices[i]);
		const vec3 near_corner_ray = vec3_mult_scalar(corner_ray, dist_min_01);
		const vec3 far_corner_ray = vec3_mult_scalar(corner_ray, dist_max_01);
		corners.vertices[i + 4] = vec3_add(corners.vertices[i], far_corner_ray);
		corners.vertices[i] = vec3_add(corners.vertices[i], near_corner_ray);
	}

	return corners;
}

struct frustum_corners frustum_corners_transform(const struct frustum_corners *corners, const mat4 transform)
{
	struct frustum_corners ret = { 0.0f };

	for(uint32_t i = 0; i < 8; i++) {
		const vec4 tmp = mat4_mult_vec4(transform, vec4_make_from_vec3(corners->vertices[i], 1.0f));

		ret.vertices[i] = (vec3) {
			.x = tmp.x / tmp.w,
			.y = tmp.y / tmp.w,
			.z = tmp.z / tmp.w
		};
	}

	return ret;
}


vec3 frustum_corners_calc_center(const struct frustum_corners *corners)
{
	vec3 center = { 0 };

	for(uint32_t i = 0; i < 8; i++) {
		center = vec3_add(center, corners->vertices[i]);
	}

	return vec3_div_scalar(center, 8);
}

struct aabb frustum_corners_calc_bounds(const struct frustum_corners *corners)
{
	struct aabb bounds = {
		.min = { FLT_MAX },
		.max = { -FLT_MAX }
	};

	for(uint32_t i = 0; i < 8; i++) {
		const vec3 *vertex = &corners->vertices[i];
		bounds.min = vec3_min(*vertex, bounds.min);
		bounds.max = vec3_max(*vertex, bounds.max);
	}

	return bounds;
}

vec3* frustum_corners_get(struct frustum_corners *corners, uint32_t x, uint32_t y, uint32_t z)
{
	return &corners->vertices[x + 2 * (y + 2 * z)];
}