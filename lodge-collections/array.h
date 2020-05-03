#ifndef _ARRAY_H
#define _ARRAY_H

#include <stddef.h>

struct array
{
	size_t	element_size;
	size_t	count;
	size_t	max_count;
	char	*data;
};

typedef struct array* array_t;

struct array*	array_new(size_t element_size, size_t max_count);
void			array_free(struct array *array);

array_t			array_create(size_t element_size, size_t max_count);
void			array_destroy(array_t array);

void*			array_get(array_t array, size_t index);
void*			array_first(array_t array);
void*			array_last(array_t array);

void			array_set(array_t array, size_t index, const void *data);
void*			array_append_no_init(array_t array);
void			array_append(array_t array, const void* data);
void			array_remove(array_t array, size_t index);
void			array_clear(array_t array);

size_t			array_count(array_t array);
size_t			array_byte_size(array_t array);
int				array_empty(array_t array);

void			array_sort(array_t array, int(*compar)(const void *, const void*));

void*			array_find(array_t array, const void *data);
int				array_equals_at(array_t array, size_t index, const void *data);
int				array_find_string(array_t array, const char *s, size_t s_len);

#define			array_create_type(type, count) \
	array_create(sizeof(type), count);

#define			array_foreach(arr, type, item) \
	for(type* item = (type*)&arr->data[0]; \
		item < &arr->data[arr->element_size * arr->count]; \
		item+=arr->element_size)

#endif
