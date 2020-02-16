#include "vertex_buffer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GL/glew.h>

#include "array.h"

struct vertex_buffer {
	struct array					*buffer;
	struct vertex_buffer_attribs	attribs;
	size_t							element_size;
};

struct vertex_buffer* vertex_buffer_new(struct vertex_buffer_attribs attribs, size_t max_elements)
{
	struct vertex_buffer *vb = (struct vertex_buffer*) malloc(sizeof(struct vertex_buffer));

	vb->attribs = attribs;

	vb->element_size = 0;
	for(int i = 0; i < VERTEX_BUFFER_ATTRIBS_MAX; i++)
	{
		vb->element_size += vb->attribs.attribs[i];
	}

	vb->buffer = array_create(vb->element_size, max_elements);

	return vb;
}

void vertex_buffer_free(struct vertex_buffer* vb)
{
	array_destroy(vb->buffer);
	free(vb);
}

void vertex_buffer_clear(struct vertex_buffer *vb)
{
	array_clear(vb->buffer);
}

const char* vertex_buffer_get_data(struct vertex_buffer* vb)
{
	return array_first(vb->buffer);
}

size_t vertex_buffer_get_data_size(struct vertex_buffer* vb)
{
	return array_count(vb->buffer) * vb->element_size;
}

void vertex_buffer_append(struct vertex_buffer *vb, const void *element, size_t element_size)
{
	assert(vb->element_size == element_size);
	array_append(vb->buffer, element);
}

struct vertex_buffer_attribs vertex_buffer_get_attribs(struct vertex_buffer *vb)
{
	return vb->attribs;
}

size_t vertex_buffer_get_element_size(struct vertex_buffer *vb)
{
	return vb->element_size;
}