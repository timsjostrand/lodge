#ifndef _LODGE_STATIC_MESH_H
#define _LODGE_STATIC_MESH_H

#include "math4.h"
#include "strview.h"

#include <stdint.h>
#include <stddef.h>

///// BUFFER OBJECT ////

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

lodge_buffer_object_t		lodge_buffer_object_make_dynamic(size_t max_size);
lodge_buffer_object_t		lodge_buffer_object_make_static(const void *data, size_t data_size);
void						lodge_buffer_object_reset(lodge_buffer_object_t buffer_object);
void						lodge_buffer_object_set(lodge_buffer_object_t buffer_object, size_t offset, const void *data, size_t data_size);

///// STATIC MESH /////

struct lodge_static_mesh
{
	lodge_buffer_object_t	vertices;
	lodge_buffer_object_t	normals;
	lodge_buffer_object_t	tex_coords;
	lodge_buffer_object_t	indices;
};

struct lodge_static_mesh	lodge_static_mesh_make(const vec3 *vertices, size_t vertices_count, const vec3 *normals, size_t normals_count, const vec2 *tex_coords, size_t tex_coords_count, const uint32_t *indices, size_t indices_count);
void						lodge_static_mesh_reset(struct lodge_static_mesh *static_mesh);

///// DRAWABLE ////

struct lodge_drawable;
typedef struct lodge_drawable* lodge_drawable_t;

#define LODGE_DRAWABLE_ATTRIBS_MAX 32

struct lodge_drawable_attrib
{
	strview_t						name;
	lodge_buffer_object_t			buffer_object;
	uint32_t						float_count;
	uint32_t						stride;
	uint32_t						instanced;
};

struct lodge_drawable_desc
{
	lodge_buffer_object_t			indices;
	struct lodge_drawable_attrib	attribs[LODGE_DRAWABLE_ATTRIBS_MAX];
	uint32_t						attribs_count;
};

lodge_drawable_t			lodge_drawable_make(struct lodge_drawable_desc desc);
lodge_drawable_t			lodge_drawable_make_from_static_mesh(const struct lodge_static_mesh *static_mesh);
void						lodge_drawable_reset(lodge_drawable_t drawable); 
void						lodge_drawable_set_index_buffer(lodge_drawable_t drawable, lodge_buffer_object_t index_buffer);
void						lodge_drawable_set_buffer_object(lodge_drawable_t drawable, uint32_t index, struct lodge_drawable_attrib attrib);

void						lodge_drawable_render_lines(const lodge_drawable_t drawable, size_t offset, size_t count);
void						lodge_drawable_render_indexed_instanced(const lodge_drawable_t drawable, size_t index_count, size_t instances);

#endif
