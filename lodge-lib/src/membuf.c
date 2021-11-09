#include "membuf.h"

#include "lodge_assert.h"
#include "lodge_platform.h"
#include "math4.h"

#include <string.h>

#if 0
static inline void* membuf_tail(membuf_t buf)
{
	return buf.ptr + buf.size;
}
#endif

membuf_t membuf_make(char *ptr, size_t size, size_t type_size)
{
	return (membuf_t) {
		.ptr = ptr,
		.size = size,
		.type_size = type_size,
	};
}

size_t membuf_size(const membuf_t buf)
{
	return buf.size;
}

size_t membuf_type_size(const membuf_t buf)
{
	return buf.type_size;
}

size_t membuf_max_count(const membuf_t buf)
{
	return buf.size / buf.type_size;
}

bool membuf_equals(const membuf_t lhs, const membuf_t rhs)
{
	if(lhs.size != rhs.size) {
		return false;
	}

	if(lhs.type_size != rhs.type_size) {
		return false;
	}

	return memcmp(lhs.ptr, rhs.ptr, lhs.size) == 0;
}

#if 0
size_t membuf_insert(membuf_t buf, size_t index, const membuf_t sub)
{
	ASSERT_NOT_IMPLEMENTED();
	return 0;
}
#endif

#if 0
size_t membuf_delete(membuf_t buf, size_t index, size_t count, size_t *current_count)
{
	const size_t type_size = buf.type_size;
	const size_t max_count = membuf_max_count(buf);

	//
	// TODO(TS): use and update current_count to not memcpy more memory than required
	//

	//
	// TODO(TS): optimize if we do not need to memcpy (something like if (index + count) == current_count )
	//

	char *delete_start = membuf_get(buf, index);
	const char *tail = membuf_tail(buf);
	const char *delete_end = membuf_get(buf, index + count);
	if(delete_end == NULL) {
		delete_end = tail;
	}

	ASSERT(delete_end < tail);

	const delete_size = tail - delete_end;

	memcpy(delete_start, delete_end, delete_size);

	return (delete_end - delete_start) / buf.type_size;
}
#endif

void membuf_swap(membuf_t buf, size_t index_a, size_t index_b)
{
	if(index_a == index_b) {
		return;
	}

	char* tmp = (char*)LODGE_ALLOCA(buf.type_size);
	ASSERT(tmp);
	char* a = membuf_get(buf, index_a);
	ASSERT(a);
	char* b = membuf_get(buf, index_b);
	ASSERT(b);

	memcpy(tmp, a, buf.type_size);
	memcpy(a, b, buf.type_size);
	memcpy(b, tmp, buf.type_size);
}

size_t membuf_delete(membuf_t buf, size_t *current_count, size_t index, size_t count)
{
	if(count < 1 || buf.size == 0 || buf.ptr == NULL) {
		return 0;
	}
	const size_t max_count = *current_count;
	if(index >= max_count) {
		return 0;
	}
	if(index+count >= max_count) {
		count = max_count - index;
	}
	/* Shift left */
	void *dst = membuf_get(buf, index);
	const void *src = membuf_get(buf, index+count);
	size_t size = (max_count - index - count) * buf.type_size;
	memmove(dst, src, size);
	*current_count -= count;
	return count;
}

struct membuf_swapret membuf_delete_swap_tail(membuf_t buf, size_t *current_count, size_t index)
{
	size_t index_b = index;

	if(*current_count > 1)
	{
		index_b = (*current_count - 1);

		ASSERT(index_b < membuf_max_count(buf));

		//
		// TODO(TS): optimize, we only need a one way swap
		//
		membuf_swap(buf, index, index_b);
	}

	--(*current_count);
	return (struct membuf_swapret) { index, index_b };
}

void* membuf_append_no_init(membuf_t dst, size_t *current_count)
{
	const size_t max_count = membuf_max_count(dst);
	ASSERT_OR(*current_count < max_count) {
		return NULL;
	}
	return membuf_get(dst, (*current_count)++);
}

void* membuf_append(membuf_t dst, size_t *current_count, const void *src, size_t src_size)
{
	const size_t max_count = membuf_max_count(dst);
	ASSERT_OR(*current_count < max_count) {
		return NULL;
	}
	return membuf_set(dst, (*current_count)++, src, src_size);
}

void* membuf_append_range(membuf_t dst, size_t *current_count, const void *src, size_t src_type_size, size_t src_count)
{
	if(!src) {
		return NULL;
	}
	ASSERT_OR(dst.type_size == src_type_size) {
		return NULL;
	}
	const size_t max_count = membuf_max_count(dst);
	ASSERT_OR((*current_count + src_count) < max_count) {
		return NULL;
	}
	void *tail = membuf_get(dst, *current_count);
	memcpy(tail, src, src_count * src_type_size);
	*current_count += src_count;
	return tail;
}

void membuf_fill(membuf_t dst, const void *src, size_t src_size)
{
	ASSERT(dst.type_size == src_size);

	const size_t type_size = dst.type_size;
	const size_t max_count = membuf_max_count(dst);
	for(size_t i = 0; i < max_count; i++) {
		memcpy(dst.ptr + i * type_size, src, src_size);
	}
}

void* membuf_get(membuf_t buf, size_t index)
{
	const size_t max_count = membuf_max_count(buf);
	ASSERT(index < max_count);
	if(index >= max_count) {
		return NULL;
	}
	ASSERT(buf.ptr);
	return buf.ptr + (index * buf.type_size);
}

void* membuf_set(membuf_t buf, size_t index, const void *src, size_t src_size)
{
	ASSERT(buf.type_size == src_size);
	if(buf.type_size != src_size) {
		return NULL;
	}

	void* dst = membuf_get(buf, index);
	ASSERT(dst);
	if(!dst) {
		return NULL;
	}

	memcpy(dst, src, src_size);
	return dst;
}

#if 0
int64_t membuf_find_ptr(membuf_t dst, const void *needle_ptr)
{
	
}
#endif

int64_t membuf_find(membuf_t dst, const size_t current_count, const void *needle, size_t needle_size)
{
	ASSERT(needle_size == dst.type_size);
	if(needle_size != dst.type_size) {
		return -1;
	}
	for(size_t i = 0; i < current_count; i++) {
		if(memcmp(membuf_get(dst, i), needle, needle_size) == 0) {
			return i;
		}
	}
	return -1;
}
