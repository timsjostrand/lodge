#ifndef _VERTEX_BUFFER_H
#define _VERTEX_BUFFER_H

#include "strview.h"
#include "lodge_platform.h"

#include <stddef.h>

#define VERTEX_BUFFER_ATTRIBS_MAX 32

struct vertex_buffer_attrib
{
	strview_t	name;
	size_t		offset;
	size_t		size;
};

#define vertex_buffer_attrib_make(NAME, TYPE, MEMBER) \
	(struct vertex_buffer_attrib) { .name = NAME, .offset = offsetof(TYPE, MEMBER), .size = sizeof_member(TYPE, MEMBER) }

struct vertex_buffer_attribs
{
	struct vertex_buffer_attrib per_vertex[VERTEX_BUFFER_ATTRIBS_MAX];
	struct vertex_buffer_attrib per_instance[VERTEX_BUFFER_ATTRIBS_MAX];
};

struct vertex_buffer;

/**
 * Example:
 *
 *		struct vertex_buffer_attribs attribs = {
 *			.per_vertex = {
 *				vertex_buffer_attrib_make("pos", struct vertex, pos),
 *				vertex_buffer_attrib_make("uv", struct vertex, uv),
 *				vertex_buffer_attrib_make("tan", struct vertex, tan),
 *				vertex_buffer_attrib_make("bitan", struct vertex, bitan),
 *			}
 *		};
 *
 *		struct vertex_buffer *vb = vertex_buffer_new(attribs, 100);
 *		vertex_buffer_append(vb, &vertex_make(pos, uv, tangent, bitangent), sizeof(vertex));
 *		vertex_buffer_append(vb, &vertex_make(pos2, uv, tangent, bitangent), sizeof(vertex));
 *		vertex_buffer_free(vb);
 *
 */
struct vertex_buffer*			vertex_buffer_new(struct vertex_buffer_attribs attribs, size_t max_elements);
void							vertex_buffer_free(struct vertex_buffer *vb);

void							vertex_buffer_append(struct vertex_buffer *vb, const void *element, size_t element_size);
void							vertex_buffer_clear(struct vertex_buffer *vb);

const char*						vertex_buffer_get_data(const struct vertex_buffer *vb);
size_t							vertex_buffer_get_data_size(const struct vertex_buffer *vb);
size_t							vertex_buffer_get_element_count(const struct vertex_buffer *vb);

struct vertex_buffer_attribs	vertex_buffer_get_attribs(const struct vertex_buffer *vb);
size_t							vertex_buffer_get_element_size(const struct vertex_buffer *vb);

#endif