#ifndef _GEOMETRY_H
#define _GEOMETRY_H

// FIXME(TS): min/max instead?
struct rect {
	vec2	pos;
	vec2	size;
};

struct circle {
	vec2	pos;
	float	r;
};

struct triangle {
	vec3	p0;
	vec3	p1;
	vec3	p2;
};

struct ray {
	vec3	origin;
	vec3	dir;
	vec3	dir_inv;
};

struct aabb {
	vec3	min;
	vec3	max;
};

#endif
