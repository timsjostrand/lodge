#ifndef _LODGE_PERSPECTIVE_H
#define _LODGE_PERSPECTIVE_H

#include "math4.h"

struct lodge_perspective
{
	float	fov_degrees;
	float	z_near;
	float	z_far;
};

mat4 lodge_perspective_calc_projection(const struct lodge_perspective *perspective, float aspect_ratio);

#endif
