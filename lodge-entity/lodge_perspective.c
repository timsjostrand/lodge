#include "lodge_perspective.h"

mat4 lodge_perspective_calc_projection(const struct lodge_perspective *p, float aspect_ratio)
{
	return mat4_perspective((float)radians(p->fov_degrees), aspect_ratio, p->z_near, p->z_far);
}
