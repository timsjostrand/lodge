#ifndef _VERTEX_H
#define _VERTEX_H

#include "math4.h"

union vertex {
	struct {
		vec3	pos;
		vec2	uv;
		vec3	tangent;
		vec3	bitangent;
	};
	struct {
		float	x;
		float	y;
		float	z;
		float	u;
		float	v;
		float	tanx;
		float	tany;
		float	tanz;
		float	bitanx;
		float	bitany;
		float	bitanz;
	};
	float ptr[11];
};
typedef union vertex vertex_t;

void vertex_calc_tangents(vertex_t *t1, vertex_t *t2, vertex_t *t3);

#endif