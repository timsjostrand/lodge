#ifndef _LODGE_STATIC_MESH_H
#define _LODGE_STATIC_MESH_H

#include "math4.h"

#include <stddef.h>

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

struct lodge_static_mesh
{
	lodge_buffer_object_t	vertices;
	lodge_buffer_object_t	normals;
	lodge_buffer_object_t	tex_coords;
	lodge_buffer_object_t	indices;
	size_t					indices_count;
};

struct lodge_static_mesh	lodge_static_mesh_make(const vec3 *vertices, size_t vertices_count, const vec3 *normals, size_t normals_count, const vec2 *tex_coords, size_t tex_coords_count, const uint32_t *indices, size_t indices_count);
void						lodge_static_mesh_reset(struct lodge_static_mesh *static_mesh);

#endif
