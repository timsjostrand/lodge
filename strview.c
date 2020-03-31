#include "strview.h"

#include "str.h"
#include "lodge_assert.h"

#include <string.h>

strview_t strview_make(const char *s, size_t length)
{
	strview_t tmp = {
		.length = length,
		.s = s
	};
	return tmp;
}

int strview_equals(const strview_t lhs, const strview_t rhs)
{
	return str_equals(lhs.s, lhs.length, rhs.s, rhs.length);
}

int strview_empty(const strview_t str)
{
	return strview_length(str) == 0;
}

int strview_length(const strview_t str)
{
	ASSERT(str.length == strlen(str.s));
	return str.length;
}