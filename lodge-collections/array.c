#include "array.h"

#include <stdlib.h>
#include <string.h>

#include "lodge_assert.h"
#include "math4.h"

struct array* array_new(size_t element_size, size_t max_count)
{
	struct array* arr = (struct array*)malloc(sizeof(struct array));
	arr->data = (char*)malloc(element_size * max_count);
	arr->element_size = element_size;
	arr->max_count = max_count;
	arr->count = 0;

	return arr;
}

void array_free(struct array* array)
{
	free(array->data);
	array->data = NULL;
	free(array);
}

array_t array_create(size_t element_size, size_t max_count)
{
	//ASSERT_FAIL("deprecated");
	return array_new(element_size, max_count);
}

void array_destroy(array_t array)
{
	//ASSERT_FAIL("deprecated");
	array_free(array);
}

void* array_get(array_t array, size_t index)
{
	ASSERT(index < array_count(array));
	return (void*)&array->data[array->element_size * index];
}

void array_set(array_t array, size_t index, const void *data)
{
	ASSERT(index < array->max_count);

	memcpy(&array->data[array->element_size * index],
		(const char*)data,
		array->element_size);

	array->count = max(array->count, index);
}

void* array_append_no_init(array_t array)
{
	ASSERT(array->data);
	ASSERT(array->count + 1 <= array->max_count);

	char* tail = array->data;
	tail += array->element_size * array->count;
	array->count++;

	return tail;
}

void array_append(array_t array, const void* data)
{
	void* element = array_append_no_init(array);
	if(element) {
		memcpy((char*)element, (const char*)data, array->element_size);
	}
}

void array_remove(array_t array, size_t index)
{
	ASSERT(index < array->count);
	memcpy((char*)&array->data[array->element_size * index],
		(char*)&array->data[array->element_size * --array->count],
		array->element_size);
}

size_t array_count(array_t array)
{
	return array->count;
}

size_t array_byte_size(array_t array)
{
	return array->element_size * array->max_count;
}

int array_empty(array_t array)
{
	return array_count(array) == 0;
}

void array_clear(array_t array)
{
	array->count = 0;
}

void* array_first(array_t array)
{
	return array_get(array, 0);
}

void* array_last(array_t array)
{
	return array_get(array, array_count(array) - 1);
}

void array_sort(array_t array, int(*compar)(const void *, const void*))
{
	qsort(array->data, array->count, array->element_size, compar);
}

int array_equals_at(array_t array, size_t index, const void *data)
{
	const void *src = array_get(array, index);
	return memcmp(src, data, array->element_size) == 0;
}

void* array_find(array_t array, const void *data)
{
	for(size_t i = 0, count = array_count(array); i < count; i++) {
		if(array_equals_at(array, i, data)) {
			return array_get(array, i);
		}
	}
	return NULL;
}

int array_find_string(array_t array, const char *s, size_t s_len)
{
	for(size_t i=0, count=array->count; i<count; i++)
	{
		const char* element = array_get(array, i);
		if(strncmp(s, element, max(s_len, strlen(element))) == 0) {
			return (int)i;
		}
	}
	return -1;
}
