#include "array.h"

#include <stdlib.h>
#include <string.h>

array_t array_create(size_t element_size, int max_count)
{
	struct array* arr = (struct array*)malloc(sizeof(struct array));
	arr->data = (char*)malloc(element_size * max_count);
	arr->element_size = element_size;
	arr->max_count = max_count;
	arr->count = 0;

	return arr;
}

void array_destroy(array_t array)
{
	free(array->data);
	free(array);
}

void* array_get(array_t array, int index)
{
	return (void*)array->data[array->element_size * index];
}

void array_append(array_t array, void* data)
{
	memcpy((char*)&array->data[array->element_size * array->count++], 
		(char*)data, 
		array->element_size);
}

void array_remove(array_t array, int index)
{
	memcpy((char*)&array->data[array->element_size * index],
		(char*)&array->data[array->element_size * --array->count],
		array->element_size);
}

int array_count(array_t array)
{
	return array->count;
}

void array_clear(array_t array)
{
	array->count = 0;
}

void* array_first(array_t array)
{
	// TODO: Assert if empty

	return (void*)&array->data[0];
}

void* array_last(array_t array)
{
	// TODO: Assert if empty

	return (void*)&array->data[array->element_size * (array->count - 1)];
}

void array_sort(array_t array, int(*compar)(const void *, const void*))
{
	qsort(array->data, array->count, array->element_size, compar);
}
