#ifndef _STRVIEW_H
#define _STRVIEW_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * A temporary view of a string, including the length (excluding the null terminator).
 *
 * @see strbuf_t
 */
typedef struct strview
{
	size_t		length;	/* Length of `s`, excluding the null terminator */
	const char*	s;
} strview_t;

strview_t	strview_make(const char *s, size_t length);
strview_t	strview_make_from_str(const char *s, size_t max_count);
strview_t	strview_null();

strview_t	strview_substring(strview_t str, size_t offset, size_t count);
strview_t	strview_substring_from_start(strview_t str, size_t offset);
strview_t	strview_substring_from_end(strview_t str, size_t offset);

bool		strview_equals(const strview_t lhs, const strview_t rhs);
bool		strview_empty(const strview_t str);
size_t		strview_length(const strview_t str);

bool		strview_begins_with(const strview_t str, const strview_t begins_with);

#if 0
#define		strview_static(s) strview_make((s), sizeof((s))-1)
#else
#define		strview_static(str) (struct strview) { .s = (str), .length = sizeof((str))-1 }
#endif

#define		strview_wrap(arr) strview_make(arr, strnlen(arr, LODGE_ARRAYSIZE(arr)))

#define		STRVIEW_PRINTF_FMT "%.*s"
#define		STRVIEW_PRINTF_ARG(STRVIEW) (int)STRVIEW.length, STRVIEW.s

#endif
