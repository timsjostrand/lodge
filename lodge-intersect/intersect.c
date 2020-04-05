/**
 * Intersection algorithms.
 *
 * Auhtor: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include "intersect.h"

#include "math4.h"
#include "geometry.h"

#include <string.h>
#include <math.h>
#include <stdio.h> // FIXME(TS): debug

void ray_init(struct ray *ray, const vec3 *origin, const vec3 *dir)
{
	memcpy(ray->origin.v, origin, sizeof(vec3));
	memcpy(ray->dir.v, dir, sizeof(vec3));

	ray->dir_inv.x = 1.0f / ray->dir.x;
	ray->dir_inv.y = 1.0f / ray->dir.y;
	ray->dir_inv.z = 1.0f / ray->dir.z;
}

int intersect_ray_vs_aabb(const struct ray *ray, const struct aabb *aabb, float *t_near, float *t_far)
{
#if 0
	float t1 = (aabb->min[0] - ray->origin[0]) * ray->dir_inv[0];
	float t2 = (aabb->max[0] - ray->origin[0]) * ray->dir_inv[0];

	float tmin = min(t1, t2);
	float tmax = max(t1, t2);

	for(int i = 1; i < 3; ++i) {
		t1 = (aabb->min[i] - ray->origin[i]) * ray->dir_inv[i];
		t2 = (aabb->max[i] - ray->origin[i]) * ray->dir_inv[i];

		tmin = max(tmin, min(t1, t2));
		tmax = min(tmax, max(t1, t2));
	}

	if(t_near != NULL)
		*t_near = tmin;
	if(t_far != NULL)
		*t_far = tmax;

	return tmax > max(tmin, 0.0f);
#elif 0
	float tmin = -INFINITY, tmax = INFINITY;

	for (int i = 0; i < 3; ++i) {
		float t1 = (aabb->min[i] - ray->origin[i]) * ray->dir_inv[i];
		float t2 = (aabb->max[i] - ray->origin[i]) * ray->dir_inv[i];

		tmin = max(tmin, min(t1, t2));
		tmax = min(tmax, max(t1, t2));
	}

	return tmax > max(tmin, 0.0);
#else
	double t1 = (aabb->min.x - ray->origin.x) * ray->dir_inv.x;
	double t2 = (aabb->max.x - ray->origin.x) * ray->dir_inv.x;

	double tmin = min(t1, t2);
	double tmax = max(t1, t2);

	for (int i = 1; i < 3; ++i) {
		t1 = (aabb->min.v[i] - ray->origin.v[i]) * ray->dir_inv.v[i];
		t2 = (aabb->max.v[i] - ray->origin.v[i]) * ray->dir_inv.v[i];

		tmin = max(tmin, min(min(t1, t2), tmax));
		tmax = min(tmax, max(max(t1, t2), tmin));
	}

	*t_near = (float)tmin;
	*t_far = (float)tmax;

	return tmax > max(tmin, 0.0);
#endif
}

#define RAY_TRIANGLE_EPSILON 0.000001

int intersect_ray_vs_triangle(const struct ray *ray, const struct triangle *triangle, float *t)
{
	vec3 edge1 = vec3_sub(triangle->p1, triangle->p0);
	vec3 edge2 = vec3_sub(triangle->p2, triangle->p0);

	vec3 pvec = vec3_cross(ray->dir, edge2);

	float det = vec3_dot(edge1, pvec);

#ifdef TEST_CULL
	if(det < RAY_TRIANGLE_EPSILON) {
		return 0;
	}

	vec3 tvec = vec3_sub(orig, triangle->p0);

	float u = vec3_dot(tvec, pvec);
	if(u < 0.0f || u > det) {
		return 0;
	}

	vec3_cross(qvec, tvec, edge1);

	float v = vec3_dot(ray->dir, qvec);
	if(v < 0.0f || v > det)
	{
		return 0;
	}

	*t = vec3_dot(edge2, qvec);
	float inv_det = 1.0f / det;
	*t *= inv_det;
	//*u *= inv_det;
	//*v *= inv_det;
#else
	if(fabs(det) < RAY_TRIANGLE_EPSILON) {
		return 0;
	}

	float inv_det = 1.0f / det;

	vec3 tvec = vec3_sub(ray->origin, triangle->p0);

	float u = vec3_dot(tvec, pvec) * inv_det;
	if(u < 0.0f || u > 1.0f)
	{
		return 0;
	}

	vec3 qvec = vec3_cross(tvec, edge1);

	float v = vec3_dot(ray->dir, qvec) * inv_det;
	if(v < 0.0f || (u + v) > 1.0f) {
		return 0;
	}

	*t = vec3_dot(edge2, qvec) * inv_det;
#endif

	return 1;
}
