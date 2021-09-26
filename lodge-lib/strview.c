#include "strview.h"

#include "str.h"
#include "lodge_assert.h"
#include "lodge_hash.h"

#include <string.h>
#include <inttypes.h>

strview_t strview_make(const char *s, size_t length)
{
	return (strview_t) {
		.length = length,
		.s = s
	};
}

strview_t strview_make_from_str(const char *s, size_t max_count)
{
	return (strview_t) {
		.length = strnlen(s, max_count),
		.s = s
	};
}

strview_t strview_null()
{
	return (strview_t) {
		.length = 0,
		.s = "\0"
	};
}

bool strview_equals(const strview_t lhs, const strview_t rhs)
{
	return str_equals(lhs.s, lhs.length, rhs.s, rhs.length);
}

bool strview_empty(const strview_t str)
{
	return strview_length(str) == 0;
}

size_t strview_length(const strview_t str)
{
	// NOTE(TS): this assert is pretty devious since strlen() assumes there will
	// be a \0 terminator somewhere which may not be true, especially for text
	// loaded from files and sub stringviews.
	//ASSERT(str.length == strlen(str.s));
	return str.length;
}

bool strview_begins_with(const strview_t str, const strview_t begins_with)
{
	return str_begins_with(str.s, str.length, begins_with.s, begins_with.length);
}

bool strview_ends_with(const strview_t str, const strview_t ends_with)
{
	return str_ends_with(str.s, str.length, ends_with.s, ends_with.length);
}

strview_t strview_substring(strview_t str, size_t offset, size_t count)
{
	ASSERT(offset < str.length);
	ASSERT(offset + count < str.length);
	return strview_make(str.s + offset, count);
}

strview_t strview_substring_from_start(strview_t str, size_t offset)
{
	ASSERT(offset < str.length);
	return strview_make(str.s + offset, str.length - offset);
}

strview_t strview_substring_from_end(strview_t str, size_t offset)
{
	ASSERT(offset < str.length);
	return strview_make(str.s + str.length - offset, offset);
}

static inline bool strview_to_lu(const strview_t str, unsigned long *out)
{
	char *end_ptr = NULL;
	unsigned long tmp = strtoul(str.s, &end_ptr, 10);
	if(errno == EINVAL || errno == ERANGE) {
		return false;
	}
	if(end_ptr > str.s + str.length) {
		return false;
	}
	*out = tmp;
	return true;
}

static inline bool strview_to_umax(const strview_t str, uintmax_t max_value, uintmax_t *out)
{
	char *end_ptr = NULL;
	errno = 0;
	unsigned long long tmp = strtoull(str.s, &end_ptr, 10);
	if(errno == EINVAL || errno == ERANGE) {
		return false;
	}
	if(end_ptr > str.s + str.length) {
		return false;
	}
	if(tmp > max_value) {
		return false;
	}
	*out = tmp;
	return true;
}

bool strview_to_u32(const strview_t str, uint32_t *out)
{
	uintmax_t tmp;
	if(strview_to_umax(str, UINT32_MAX, &tmp)) {
		*out = (uint32_t)tmp;
		return true;
	}
	return false;
}

bool strview_to_u64(const strview_t str, uint64_t *out)
{
	uintmax_t tmp;
	if(strview_to_umax(str, UINT64_MAX, &tmp)) {
		*out = (uint64_t)tmp;
		return true;
	}
	return false;
}

uint32_t strview_calc_hash(const strview_t str)
{
	return lodge_hash_murmur3_32(str.s, str.length);
}