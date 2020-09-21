#ifndef _MEMBUF_H
#define _MEMBUF_H

#include <stddef.h>
#include <stdbool.h>

/**
 * A wrapper for static sized memory buffers.
 *
 * Example:
 *
 *		float my_array[] = { 1.0f, 2.0f };
 *		membuf_t my_buf = membuf_wrap(my_array);
 *		printf("%f %f\n", *(float*)membuf_get(my_buf, 0), *(float*)membuf_get(my_buf, 1));
 *		// Output: "1.0 2.0"
 *
 */
typedef struct membuf
{
	size_t		size;			// full array size
	size_t		type_size;		// individual array element size
	char*		ptr;			// ptr to array
} membuf_t;

struct membuf_swapret
{
	size_t		index_a;
	size_t		index_b;
};

// typedef const membuf_t memview_t;

membuf_t				membuf_make(char *ptr, size_t size, size_t type_size);
#define					membuf_wrap(buffer) membuf_make((char*)(buffer), sizeof((buffer)), sizeof(buffer[0]))
#define					membuf_wrap_and(arr, func, ...) func(membuf_wrap(arr), __VA_ARGS__)

size_t					membuf_size(const membuf_t buf);
size_t					membuf_type_size(const membuf_t buf);
size_t					membuf_max_count(const membuf_t buf);

void*					membuf_get(membuf_t buf, size_t index);
void*					membuf_set(membuf_t buf, size_t index, const void *src, size_t src_size);

bool					membuf_equals(const membuf_t lhs, const membuf_t rhs);
//size_t				membuf_insert(membuf_t buf, size_t index, const membuf_t sub);
//size_t				membuf_delete(membuf_t buf, size_t index, size_t count, size_t *current_count);
struct membuf_swapret	membuf_delete_swap_tail(membuf_t buf, size_t index, size_t *current_count);
void					membuf_swap(membuf_t buf, size_t index_a, size_t index_b);
void*					membuf_append(membuf_t dst, const void *src, size_t src_size, size_t *current_count);
void					membuf_fill(membuf_t dst, const void *src, size_t src_size);

//
// Returns the maximum index of the membuf (ie. max_count - 1).
//
static inline size_t membuf_max_index(membuf_t buf)
{
	return membuf_max_count(buf) - 1;
}


#endif
