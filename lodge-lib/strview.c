#include "strview.h"

#include "str.h"
#include "lodge_assert.h"

#include <string.h>

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
		.s = '\0'
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