#ifndef _STRBUF_H
#define _STRBUF_H

#include "strview.h"

#include <stddef.h>
#include <stdint.h>

/**
 * A wrapper for static sized character buffers.
 *
 * Example:
 *
 *		char my_string[1024];
 *		strbuf_t my_buf = strbuf_wrap(my_string);
 *		strbuf_append(my_buf, strview_static("Hello world"));
 *		strbuf_append(my_buf, strview_static("!"));
 *		printf("%s", my_buf.s);
 *		// Output: "Hello world!"
 *
 * Example with `strbuf_wrap_and`:
 *
 *		char my_string[1024];
 *		strbuf_wrap_and(my_string, strbuf_set, strview_static("Hello world"));
 *
 */
typedef struct strbuf
{
	size_t		size;	/* The allocated size for `s` (NOT the length!) */
	char*		s;
} strbuf_t;

strbuf_t	strbuf_make(char *s, size_t size);
#define		strbuf_wrap(buffer) strbuf_make((buffer), sizeof((buffer)))
#define		strbuf_wrap_and(arr, func, ...) func(strbuf_wrap(arr), __VA_ARGS__)
size_t		strbuf_length(const strbuf_t str);
size_t		strbuf_size(const strbuf_t str);
int			strbuf_equals(const strbuf_t lhs, const strview_t rhs);
strview_t	strbuf_to_strview(const strbuf_t str);
size_t		strbuf_insert(strbuf_t str, size_t index, const strview_t sub);
size_t		strbuf_delete(strbuf_t str, size_t index, size_t count);
size_t		strbuf_set(strbuf_t dst, const strview_t src);
size_t		strbuf_setf(strbuf_t dst, const char *fmt, ...);
size_t		strbuf_append(strbuf_t dst, const strview_t src);
void		strbuf_fill(strbuf_t dst, char c);

#endif
