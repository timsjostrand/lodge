#include "lodge_static_mesh.h"

#include "lodge_assert.h"
#include "lodge_buffer_object.h"

struct lodge_static_mesh lodge_static_mesh_make(const vec3 *vertices, size_t vertices_count, const vec3 *normals, size_t normals_count, const vec2 *tex_coords, size_t tex_coords_count, const uint32_t *indices, size_t indices_count)
{
	ASSERT(vertices_count == normals_count && vertices_count == tex_coords_count);

	return (struct lodge_static_mesh) {
		.vertices = lodge_buffer_object_make_static(vertices, vertices_count * sizeof(vec3)),
		.normals = lodge_buffer_object_make_static(normals, normals_count * sizeof(vec3)),
		.tex_coords = lodge_buffer_object_make_static(tex_coords, tex_coords_count * sizeof(vec2)),
		.indices = lodge_buffer_object_make_static(indices, indices_count * sizeof(uint32_t)),
		.indices_count = indices_count,
	};
}

void lodge_static_mesh_reset(struct lodge_static_mesh *static_mesh)
{
	lodge_buffer_object_reset(static_mesh->vertices);
	lodge_buffer_object_reset(static_mesh->normals);
	lodge_buffer_object_reset(static_mesh->tex_coords);
	lodge_buffer_object_reset(static_mesh->indices);

	*static_mesh = (struct lodge_static_mesh) { 0 };
}
