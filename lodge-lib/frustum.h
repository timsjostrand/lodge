#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "math4.h"
#include "geometry.h"

#include <stdbool.h>

#if 0

enum frustum_planes_index
{
	FRUSTUM_PLANE_LEFT = 0,
	FRUSTUM_PLANE_RIGHT = 1,
	FRUSTUM_PLANE_TOP = 2,
	FRUSTUM_PLANE_BOTTOM = 3,
	FRUSTUM_PLANE_NEAR = 4,
	FRUSTUM_PLANE_FAR = 5,
};

struct frustum_planes
{
	vec4 planes[6];
};

struct frustum_planes	frustum_planes_make_from_projection(const mat4 mat);
bool					frustum_planes_is_visible_sphere(struct frustum_planes *frustum, struct sphere sphere);

#endif

struct frustum_corners
{
	vec3 vertices[2][2][2];
};

struct frustum_corners	frustum_corners_make(const mat4 inv_view_proj);
struct frustum_corners	frustum_corners_transform(const struct frustum_corners *corners, const mat4 transform);
vec3					frustum_corners_calc_center(const struct frustum_corners *corners);
struct aabb				frustum_corners_calc_bounds(const struct frustum_corners *corners);

#endif