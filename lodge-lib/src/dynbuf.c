#include "dynbuf.h"

#include "membuf.h"

#include "lodge_assert.h"
#include "lodge_platform.h"

#include <string.h>
#include <stdlib.h>

#define DYNBUF_CAPACITY_MIN 8

static membuf_t dynbuf_to_membuf(dynbuf_t dynbuf)
{
	return membuf_make(*dynbuf.ptr, dynbuf.type_size * (*dynbuf.capacity), dynbuf.type_size);
}

static void dynbuf_maybe_grow(dynbuf_t dst, size_t new_count)
{
	ASSERT(dst.ptr);

	if(!*dst.ptr || (new_count >= *dst.capacity)) {
		size_t new_capacity = max(*dst.capacity, DYNBUF_CAPACITY_MIN);

		if(new_count >= new_capacity) {
			new_capacity <<= 1;
		}

		char *new_ptr = NULL;

		if(*dst.ptr) {
			new_ptr = realloc(*dst.ptr, new_capacity * dst.type_size);
		} else {
			new_ptr = malloc(new_capacity * dst.type_size);
		}

		ASSERT(new_ptr);

		if(new_ptr) {
			*dst.ptr = new_ptr;
			*dst.capacity = new_capacity;
		}
	}
}

void dynbuf_new_inplace(dynbuf_t dynbuf, size_t default_capacity)
{
	//
	// If this assert hits, you probably did not use `dynbuf_wrap()`.
	//
	ASSERT(dynbuf.type_size > 0);
	*dynbuf.capacity = max(default_capacity, DYNBUF_CAPACITY_MIN);
	*dynbuf.count = 0;
	*dynbuf.ptr = NULL;
}

void dynbuf_free_inplace(dynbuf_t dynbuf)
{
	if(dynbuf.ptr) {
		free(*dynbuf.ptr);
		*dynbuf.ptr = NULL;
	}
}

void dynbuf_clear(dynbuf_t dynbuf)
{
	*(dynbuf.count) = 0;
}

void* dynbuf_append(dynbuf_t dst, const void *src, size_t src_size)
{
	dynbuf_maybe_grow(dst, *dst.count + 1);
	return membuf_append(dynbuf_to_membuf(dst), dst.count, src, src_size);
}

void* dynbuf_append_no_init(dynbuf_t dst)
{
	dynbuf_maybe_grow(dst, *dst.count + 1);
	return membuf_append_no_init(dynbuf_to_membuf(dst), dst.count);
}

void* dynbuf_append_range(dynbuf_t dst, void *src, size_t src_type_size, size_t src_count)
{
	if(!src) {
		return NULL;
	}
	ASSERT_OR(src_type_size == dst.type_size) {
		return NULL;
	}
	dynbuf_maybe_grow(dst, *dst.count + src_count);
	return membuf_append_range(dynbuf_to_membuf(dst), dst.count, src, src_type_size, src_count);
}

int64_t dynbuf_find(dynbuf_t buf, const void *needle, size_t needle_size)
{
	return membuf_find(dynbuf_to_membuf(buf), *buf.count, needle, needle_size);
}

size_t dynbuf_remove(dynbuf_t buf, size_t index, size_t count)
{
	return membuf_delete(dynbuf_to_membuf(buf), buf.count, index, count);
}
