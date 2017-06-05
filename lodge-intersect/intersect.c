/**
 * Intersection algorithms.
 *
 * Auhtor: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <string.h>
#include <math.h>

#include <stdio.h> // FIXME(TS): debug

#include "intersect.h"
#include "math4.h"
#include "geometry.h"

void ray_init(struct ray *ray, const vec3 *origin, const vec3 *dir)
{
	memcpy(ray->origin, origin, sizeof(vec3));
	memcpy(ray->dir, dir, sizeof(vec3));

	ray->dir_inv[0] = 1.0f / ray->dir[0];
	ray->dir_inv[1] = 1.0f / ray->dir[1];
	ray->dir_inv[2] = 1.0f / ray->dir[2];
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
	double t1 = (aabb->min[0] - ray->origin[0]) * ray->dir_inv[0];
	double t2 = (aabb->max[0] - ray->origin[0]) * ray->dir_inv[0];

	double tmin = min(t1, t2);
	double tmax = max(t1, t2);

	for (int i = 1; i < 3; ++i) {
		t1 = (aabb->min[i] - ray->origin[i]) * ray->dir_inv[i];
		t2 = (aabb->max[i] - ray->origin[i]) * ray->dir_inv[i];

		tmin = max(tmin, min(min(t1, t2), tmax));
		tmax = min(tmax, max(max(t1, t2), tmin));
	}

	*t_near = tmin;
	*t_far = tmax;

	return tmax > max(tmin, 0.0);
#endif
}

#define RAY_TRIANGLE_EPSILON 0.000001

int intersect_ray_vs_triangle(const struct ray *ray, const struct triangle *triangle, float *t)
{
	vec3 edge1;
	vec3 edge2;
	sub3v(edge1, triangle->p1, triangle->p0);
	sub3v(edge2, triangle->p2, triangle->p0);

	vec3 pvec;
	cross(pvec, ray->dir, edge2);

	float det = dot3v(edge1, pvec);

#ifdef TEST_CULL
	if(det < RAY_TRIANGLE_EPSILON)
	{
		return 0;
	}

	vec3 tvec;
	sub3v(tvec, orig, triangle->p0);

	float u = dot3v(tvec, pvec);
	if(u < 0.0f || u > det)
	{
		return 0;
	}

	cross3v(qvec, tvec, edge1);

	float v = dot3v(ray->dir, qvec);
	if(v < 0.0f || v > det)
	{
		return 0;
	}

	*t = dot3v(edge2, qvec);
	float inv_det = 1.0f / det;
	*t *= inv_det;
	//*u *= inv_det;
	//*v *= inv_det;
#else
	if(fabs(det) < RAY_TRIANGLE_EPSILON)
	{
		return 0;
	}

	float inv_det = 1.0f / det;

	vec3 tvec;
	sub3v(tvec, ray->origin, triangle->p0);

	float u = dot3v(tvec, pvec) * inv_det;
	if(u < 0.0f || u > 1.0f)
	{
		return 0;
	}

	vec3 qvec;
	cross(qvec, tvec, edge1);

	float v = dot3v(ray->dir, qvec) * inv_det;
	if(v < 0.0f || (u + v) > 1.0f)
	{
		return 0;
	}

	*t = dot3v(edge2, qvec) * inv_det;
#endif

	return 1;
}
