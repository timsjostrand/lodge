#ifndef _VERTEX_BUFFER_H
#define _VERTEX_BUFFER_H

#include <stdint.h>

#define sizeof_member(type, member) sizeof(((type *)0)->member)

#define VERTEX_BUFFER_ATTRIBS_MAX 32

struct vertex_buffer_attribs
{
	size_t	attribs[VERTEX_BUFFER_ATTRIBS_MAX];
};

struct vertex_buffer;

/**
 * Example initialization:
 *
 *		size_t attribs[] = {
 *			sizeof_member(vertex, pos),
 *			sizeof_member(vertex, uv),
 *			sizeof_member(vertex, tangent),
 *			sizeof_member(vertex, tangent),
 *		};
 *
 *		struct vertex_buffer *vb = vertex_buffer_new(attribs, 100);
 *		vertex_buffer_append(vb, vertex_make(pos, uv, tangent, bitangent), sizeof(vertex));
 *		vertex_buffer_append(vb, vertex_make(pos2, uv, tangent, bitangent), sizeof(vertex));
 *		vertex_buffer_free(vb);
 *
 */
struct vertex_buffer*			vertex_buffer_new(struct vertex_buffer_attribs attribs, size_t max_elements);
void							vertex_buffer_free(struct vertex_buffer *vb);

void							vertex_buffer_append(struct vertex_buffer *vb, const void *element, size_t element_size);
void							vertex_buffer_clear(struct vertex_buffer *vb);

const char*						vertex_buffer_get_data(struct vertex_buffer *vb);
size_t							vertex_buffer_get_data_size(struct vertex_buffer *vb);

struct vertex_buffer_attribs	vertex_buffer_get_attribs(struct vertex_buffer *vb);
size_t							vertex_buffer_get_element_size(struct vertex_buffer *vb);

#endif