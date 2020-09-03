#ifndef _LODGE_BUFFER_OBJECT_H
#define _LODGE_BUFFER_OBJECT_H

#include <stddef.h>

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

lodge_buffer_object_t	lodge_buffer_object_make_dynamic(size_t max_size);
lodge_buffer_object_t	lodge_buffer_object_make_static(const void *data, size_t data_size);
void					lodge_buffer_object_reset(lodge_buffer_object_t buffer_object);

void					lodge_buffer_object_set(lodge_buffer_object_t buffer_object, size_t offset, const void *data, size_t data_size);

#endif