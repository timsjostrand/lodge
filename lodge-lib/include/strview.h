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
bool		strview_ends_with(const strview_t str, const strview_t ends_with);

bool		strview_to_u32(const strview_t str, uint32_t *out);
bool		strview_to_u64(const strview_t str, uint64_t *out);

uint32_t	strview_calc_hash(const strview_t str);

static inline bool strview_is_null_terminated(const strview_t str)
{
	return str.s ? (str.s[str.length] == '\0') : false;
}

#define		strview(str) (struct strview) { .s = (str), .length = sizeof((str))-1 }
#define		strview_static(str) strview(str)

#define		strview_wrap(arr) strview_make(arr, strnlen(arr, LODGE_ARRAYSIZE(arr)))

#define		STRVIEW_PRINTF_FMT "%.*s"
#define		STRVIEW_PRINTF_ARG(STRVIEW) (int)STRVIEW.length, STRVIEW.s

#define		ASSERT_NULL_TERMINATED(strview) ASSERT(strview_is_null_terminated(strview))

#endif
