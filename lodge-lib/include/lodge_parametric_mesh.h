#pragma once

#include "lodge_platform.h"

#include "math4.h"

struct lodge_parametric_mesh
{
	struct
	{
		size_t	capacity;
		size_t	count;
		vec3	*elements;
	} vertices;

	struct
	{
		size_t	capacity;
		size_t	count;
		vec3	*elements;
	} normals;

	struct
	{
		size_t	capacity;
		size_t	count;
		vec2	*elements;
	} uvs;
};

void							lodge_parametric_mesh_free_inplace(struct lodge_parametric_mesh* dst);

struct lodge_parametric_mesh	lodge_parametric_mesh_new_plane_subdivided(vec2 origin, vec2 size, uint32_t divisions_x, uint32_t divisions_y);
