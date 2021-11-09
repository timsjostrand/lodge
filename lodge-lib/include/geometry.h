#ifndef _GEOMETRY_H
#define _GEOMETRY_H

//
// FIXME(TS): should use min/max instead
//
struct rect
{
	vec2	pos;
	vec2	size;
};

struct circle
{
	vec2	pos;
	float	r;
};

struct triangle
{
	vec3	p0;
	vec3	p1;
	vec3	p2;
};

struct ray
{
	vec3	origin;
	vec3	dir;
	vec3	dir_inv;
};

struct aabb
{
	vec3	min;
	vec3	max;
};

struct line
{
	vec3	p0;
	vec3	p1;
};

struct sphere
{
	vec3	pos;
	float	r;
};

inline struct ray ray_make(const vec3 origin, const vec3 dir)
{
	return (struct ray) {
		.origin = origin,
		.dir = dir,
		.dir_inv = {
			.x = 1.0f / dir.x,
			.y = 1.0f / dir.y,
			.z = 1.0f / dir.z
		}
	};
}

#endif
