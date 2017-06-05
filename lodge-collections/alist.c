/**
 * Dynamic array list implementation.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alist.h"

/**
 * If required, grows the list in increments of 2x until the specified index
 * can be accessed.
 *
 * @param alist	The list to maybe resize.
 * @param index	The index that should be accessible.
 *
 * @return		0 on success, not 0 on error.
 */
static int alist_maybe_grow(struct alist *alist, size_t index)
{
	if(alist == NULL) {
		return -1;
	}
	size_t new_size = alist->size;
	while(index >= new_size) {
		new_size *= 2;
	}
	if(new_size != alist->size
			&& alist_resize(alist, new_size) != 0) {
		return -1;
	}
	return 0;
}

/**
 * Shift data pointers a number steps to the right. The data array will grow
 * if it cannot hold all the elements. This will also increase alist->count by
 * shift_count.
 *
 * @param alist
 * @param offset		The offset into the data array to shift from.
 * @param shift_count	The number of times to shift to the right.
 */
static int alist_shift_data_right(struct alist *alist, size_t offset, size_t shift_count)
{
	/* Sanity check. */
	if(alist == NULL) {
		return -1;
	}
	/* Nothing to do. */
	if(shift_count == 0) {
		return 0;
	}
	/* Resize data array? */
	if(alist_maybe_grow(alist, alist->count + shift_count) != 0) {
		return -1;
	}
	for(size_t i=alist->count + shift_count; i > offset && i>=shift_count; i--) {
		alist->data[i] = alist->data[i - shift_count];
	}
	/* Zero the new elements. */
	memset(alist->data + offset, 0, (shift_count) * sizeof(alist_data_t *));
	alist->count += shift_count;
	return 0;
}

/**
 * Shift data pointers a number steps to the left and decrease list->count by
 * shift_count.
 *
 * @param alist			List to shift.
 * @param offset		The offset into the data array to shift from.
 * @param shift_count	The number of times to shift to the left.
 * @return				0 on success, not 0 on error.
 */
static int alist_shift_data_left(struct alist *alist, size_t offset, size_t shift_count)
{
	/* Sanity check. */
	if(alist == NULL
			|| offset > alist->count) {
		return -1;
	}
	/* Nothing to do. */
	if(shift_count == 0) {
		return 0;
	}
	/* Out of bounds on left side: begin at start of array. */
	if(shift_count > offset) {
		offset = shift_count;
	}
	for(size_t i=(offset - shift_count); i < alist->count - shift_count; i++) {
		alist->data[i] = alist->data[i + shift_count];
	}
	/* Zero the new elements. */
	memset(alist->data + (alist->count - shift_count), 0, (shift_count) * sizeof(alist_data_t *));
	alist->count -= shift_count;
	return 0;
}

struct alist* alist_new(size_t initial_size)
{
	struct alist *tmp = (struct alist *) malloc(sizeof(struct alist));
	if(alist_init(tmp, initial_size) != 0) {
		goto cleanup;
	}
	return tmp;
cleanup:
	free(tmp);
	return NULL;
}

static void alist_free_data_elements(struct alist *alist)
{
	foreach_alist(void *, e, index, alist) {
		free(e);
	}
}

int alist_init(struct alist *alist, size_t initial_size)
{
	if(alist == NULL) {
		return -1;
	}

	/* Sanity check. */
	if(initial_size == 0) {
		return -1;
	}

	alist->data = (alist_data_t *) malloc(initial_size * sizeof(alist_data_t *));

	if(alist->data == NULL) {
		return -1;
	}

	alist->size = initial_size;
	alist->count = 0;

	return 0;
}

void alist_free(struct alist *alist, int free_data)
{
	if(free_data) {
		alist_free_data_elements(alist);
	}
	free(alist->data);
	free(alist);
}

int alist_resize(struct alist *alist, size_t new_size)
{
	alist_data_t *tmp = (alist_data_t *) realloc(alist->data, new_size * sizeof(alist_data_t *));
	if(tmp == NULL) {
		return -1;
	}
	alist->data = tmp;
	alist->size = new_size;
	return 0;
}

int alist_append(struct alist *alist, alist_data_t *data)
{
	if(alist_maybe_grow(alist, alist->count + 1) != 0) {
		return -1;
	}
	alist->data[alist->count++] = data;
	return 0;
}

void alist_clear(struct alist *alist, int free_data)
{
	if(free_data) {
		alist_free_data_elements(alist);
	}
	alist->count = 0;
}

int alist_prepend(struct alist *alist, alist_data_t *data)
{
	return alist_insert_at(alist, 0, data);
}

int alist_insert_at(struct alist *alist, size_t index, alist_data_t *data)
{
	if(alist_shift_data_right(alist, index, 1) != 0) {
		return -1;
	}
	alist->data[index] = data;
	return 0;
}

alist_data_t* alist_get(struct alist *alist, size_t index)
{
	/* Sanity check. */
	if(alist == NULL) {
		return NULL;
	}
	/* Out of bounds. */
	if(index >= alist->count) {
		return NULL;
	}
	return alist->data[index];
}

/**
 * @return	Returns a pointer to the deleted item (or if the free_data flag is set:
 *			where the data used to be). Returns NULL on error.
 */
alist_data_t* alist_delete_at(struct alist *alist, size_t index, int free_data)
{
	alist_data_t *tmp = alist_get(alist, index);
	if(tmp != NULL) {
		if(free_data) {
			free(tmp);
		}
	}
	alist_shift_data_left(alist, index + 1, 1);
	return tmp;
}

int alist_empty(struct alist *alist)
{
	return alist->count == 0;
}

size_t alist_count(struct alist *alist)
{
	return alist->count;
}

alist_data_t* alist_first(struct alist *alist)
{
	if(alist->count == 0) {
		return NULL;
	}
	return alist->data[0];
}

alist_data_t* alist_last(struct alist *alist)
{
	if(alist->count == 0) {
		return NULL;
	}
	return alist->data[alist->count - 1];
}

/**
 * Searches a list for the first matching data element. If it is found, it is
 * stored in 'index' and the function returns 0. Otherwise, the function will
 * return another value.
 *
 * The function does not return the index directly due to size_t being unsigned
 * and error reporting would not be possible.
 *
 * @param alist		The list to search in.
 * @param data		The data pointer to look for.
 * @param index		Where to store the index, if found.
 *
 * @return			0 on success, not 0 on error.
 */
int alist_index_of(struct alist *alist, alist_data_t *data, size_t *index)
{
	for(size_t i=0; i<alist->count; i++) {
		if(alist->data[i] == data) {
			(*index) = i;
			return 0;
		}
	}
	return -1;
}

/**
 * A debug function that assumes all pointers in a list are const char
 * pointers, and prints them to stdout with the format "index=string".
 */
void alist_print_strings(struct alist *alist)
{
	foreach_alist(const char *, s, index, alist) {
		printf("%lu=%s\n", index, s);
	}
}

struct alist* alist_copy_subset(struct alist *original, size_t start,
		size_t count)
{
	/* Sanity check. */
	if(original == NULL) {
		return NULL;
	}
	/* Out of bounds on right? */
	if(start > original->count) {
		return NULL;
	}
	/* Out of bounds on left? */
	if(count > original->count) {
		count = original->count;
	}
	struct alist *tmp = alist_new(original->size);
	memcpy(tmp->data, original->data + start, count * sizeof(alist_data_t *));
	tmp->count = count;
	return tmp;
}

struct alist* alist_copy(struct alist *original)
{
	return alist_copy_subset(original, 0, original->count);
}
