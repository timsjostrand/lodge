#include "lodge_parametric_drawable.h"

#include "lodge_drawable.h"
#include "lodge_buffer_object.h"
#include "lodge_parametric_mesh.h"

static lodge_drawable_t lodge_drawable_make_from_parametric(const struct lodge_parametric_mesh *p)
{
	return lodge_drawable_make((struct lodge_drawable_desc) {
		.indices = NULL,
		.attribs_count = 3,
		.attribs = {
			{
				.name = strview_static("vertex"),
				.buffer_object = lodge_buffer_object_make_static(p->vertices.elements, p->vertices.count * sizeof(vec3)),
				.float_count = 3,
				.stride = sizeof(vec3),
				.instanced = 0,
			},
			{
				.name = strview_static("normal"),
				.buffer_object = lodge_buffer_object_make_static(p->normals.elements, p->normals.count * sizeof(vec3)),
				.float_count = 3,
				.stride = sizeof(vec3),
				.instanced = 0,
			},
			{
				.name = strview_static("tex_coord"),
				.buffer_object = lodge_buffer_object_make_static(p->uvs.elements, p->uvs.count * sizeof(vec2)),
				.float_count = 2,
				.stride = sizeof(vec2),
				.instanced = 0,
			}
		}
	});
}

lodge_drawable_t lodge_drawable_make_plane_subdivided(vec2 origin, vec2 size, uint32_t divisions_x, uint32_t divisions_y)
{
	struct lodge_parametric_mesh tmp = lodge_parametric_mesh_new_plane_subdivided(origin, size, divisions_x, divisions_y);
	lodge_drawable_t drawable = lodge_drawable_make_from_parametric(&tmp);
	ASSERT(drawable);
	lodge_parametric_mesh_free_inplace(&tmp);
	return drawable;
}