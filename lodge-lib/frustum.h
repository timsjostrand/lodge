#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "math4.h"
#include "geometry.h"

#include <stdbool.h>

enum frustum_planes_index
{
	FRUSTUM_PLANE_LEFT = 0,
	FRUSTUM_PLANE_RIGHT = 1,
	FRUSTUM_PLANE_TOP = 2,
	FRUSTUM_PLANE_BOTTOM = 3,
	FRUSTUM_PLANE_NEAR = 4,
	FRUSTUM_PLANE_FAR = 5,
	FRUSTUM_PLANE_MAX = 6,
};

struct frustum_planes
{
	vec4 planes[6];
};

struct frustum_planes	frustum_planes_make(const mat4 view_proj);
bool					frustum_planes_vs_aabb(struct frustum_planes *frustum, struct aabb aabb);
bool					frustum_planes_vs_sphere(struct frustum_planes *frustum, struct sphere *sphere);

struct frustum_corners
{
	vec3 vertices[8];
};

struct frustum_corners	frustum_corners_make(const mat4 inv_view_proj);
struct frustum_corners	frustum_corners_make_slice(const mat4 inv_view_proj, float dist_min_01, float dist_max_01);
struct frustum_corners	frustum_corners_transform(const struct frustum_corners *corners, const mat4 transform);
vec3					frustum_corners_calc_center(const struct frustum_corners *corners);
struct aabb				frustum_corners_calc_bounds(const struct frustum_corners *corners);

vec3*					frustum_corners_get(struct frustum_corners *corners, uint32_t x, uint32_t y, uint32_t z);

#endif