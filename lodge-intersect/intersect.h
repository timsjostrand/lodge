#ifndef _INTERSECT_H
#define _INTERSECT_H

#include "math4.h"
#include "geometry.h"

void ray_init(struct ray *ray, const vec3 *origin, const vec3 *dir);

int intersect_ray_vs_aabb(const struct ray *ray, const struct aabb *aabb, float *t_near, float *t_far);
int intersect_ray_vs_triangle(const struct ray *ray, const struct triangle *triangle, float *t);

#endif
