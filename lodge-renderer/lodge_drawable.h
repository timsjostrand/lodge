#ifndef _LODGE_DRAWABLE_H
#define _LODGE_DRAWABLE_H

#include "strview.h"

#include <stdint.h>

#define LODGE_DRAWABLE_ATTRIBS_MAX	32

struct lodge_drawable;
typedef struct lodge_drawable* lodge_drawable_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

struct lodge_drawable_attrib
{
	strview_t						name;
	lodge_buffer_object_t			buffer_object;
	uint32_t						float_count;
	uint32_t						offset;
	uint32_t						stride;
	uint32_t						instanced;
};

struct lodge_drawable_desc
{
	lodge_buffer_object_t			indices;
	struct lodge_drawable_attrib	attribs[LODGE_DRAWABLE_ATTRIBS_MAX];
	uint32_t						attribs_count;
};

struct lodge_drawable_desc			lodge_drawable_desc_make_from_static_mesh(const struct lodge_static_mesh *static_mesh);

lodge_drawable_t					lodge_drawable_make(struct lodge_drawable_desc desc);
lodge_drawable_t					lodge_drawable_make_from_static_mesh(const struct lodge_static_mesh *static_mesh);
void								lodge_drawable_reset(lodge_drawable_t drawable); 
void								lodge_drawable_set_index_buffer(lodge_drawable_t drawable, lodge_buffer_object_t index_buffer);
void								lodge_drawable_set_buffer_object(lodge_drawable_t drawable, uint32_t index, struct lodge_drawable_attrib attrib);

void								lodge_drawable_render_lines(const lodge_drawable_t drawable, size_t offset, size_t count);
void								lodge_drawable_render_triangles(const lodge_drawable_t drawable, size_t offset, size_t count);
void								lodge_drawable_render_indexed_instanced(const lodge_drawable_t drawable, size_t index_count, size_t instances);
void								lodge_drawable_render_indexed(const lodge_drawable_t drawable, size_t index_count);

#endif