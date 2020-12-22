#include "lodge_perspective.h"

mat4 lodge_perspective_calc_projection(const struct lodge_perspective *p)
{
	const float ratio = p->width / p->height;
	return mat4_perspective((float)radians(p->fov_degrees), ratio, p->z_near, p->z_far);
}
