#ifndef _STRVIEW_H
#define _STRVIEW_H

#include <stdint.h>

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
int			strview_equals(const strview_t lhs, const strview_t rhs);
int			strview_empty(const strview_t str);
int			strview_length(const strview_t str);
#define		strview_static(s) strview_make((s), sizeof((s))-1)

#define		STRVIEW_PRINTF_FMT "%.*s"
#define		STRVIEW_PRINTF_ARG(STRVIEW) STRVIEW.length, STRVIEW.s

#endif