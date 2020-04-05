#include "blob_cur.h"

#include "math4.h"
#include "lodge_assert.h"

#include <string.h>

struct blob_cur blob_cur_make(const char* buf, size_t buf_size)
{
	return (struct blob_cur) {
		.begin = buf,
		.it = buf,
		.end = buf + buf_size
	};
}

struct blob_cur blob_cur_make_from_cur(const struct blob_cur *cur, size_t sub_size)
{
	ASSERT(blob_cur_can_read(cur, sub_size));

	return (struct blob_cur) {
		.begin = cur->begin,
		.it = cur->it,
		.end = min(cur->it + sub_size, cur->end)
	};
}

struct blob_cur blob_cur_make_from_start(const struct blob_cur *cur, size_t offset)
{
	ASSERT(blob_cur_contains(cur, offset));

	return (struct blob_cur) {
		.begin = cur->begin,
		.it = cur->it,
		.end = min(cur->begin + offset, cur->end)
	};
}

size_t blob_cur_size(const struct blob_cur *cur)
{
	return cur->end - cur->begin;
}

size_t blob_cur_remaining(const struct blob_cur *cur)
{
	return cur->end - cur->it;
}

int blob_cur_can_read(const struct blob_cur *cur, size_t size)
{
	return (cur->it + size) <= cur->end;
}

int blob_cur_contains(const struct blob_cur *cur, size_t offset)
{
	return (cur->begin + offset) <= cur->end;
}

int blob_cur_is_empty(const struct blob_cur *cur)
{
	return cur->it >= cur->end;
}

int blob_cur_read_type(char* dst, size_t dst_size, struct blob_cur *cur)
{
	if(!blob_cur_can_read(cur, dst_size)) {
		ASSERT_FAIL("Data cursor underrun");
		return 0;
	}
	memcpy(dst, cur->it, dst_size);
	cur->it += dst_size;
	return 1;
}

size_t blob_cur_offset(const struct blob_cur *cur)
{
	return cur->it - cur->begin;
}

int blob_cur_advance(struct blob_cur *dst, size_t offset)
{
	if(!blob_cur_contains(dst, (size_t)(dst->it - dst->begin) + offset)) {
		return 0;
	}
	dst->it += offset;
	return 1;
}

int blob_cur_mirror(struct blob_cur *dst, const struct blob_cur *src)
{
	if(dst->begin != src->begin) {
		return 0;
	}
	if(!blob_cur_contains(dst, blob_cur_offset(src))) {
		return 0;
	}
	dst->it = src->it;
	return 1;
}
