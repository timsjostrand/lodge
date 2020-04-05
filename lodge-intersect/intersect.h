#ifndef _INTERSECT_H
#define _INTERSECT_H

struct ray;
struct aabb;
struct triangle;

int intersect_ray_vs_aabb(const struct ray *ray, const struct aabb *aabb, float *t_near, float *t_far);
int intersect_ray_vs_triangle(const struct ray *ray, const struct triangle *triangle, float *t);

#endif
