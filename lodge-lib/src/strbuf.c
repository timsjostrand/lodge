#include "strbuf.h"

#include "str.h"

strbuf_t strbuf_make(char *s, size_t size)
{
	strbuf_t tmp = {
		.size = size,
		.s = s
	};
	return tmp;
}

strview_t strbuf_to_strview(const strbuf_t str)
{
	return strview_make(str.s, strbuf_length(str));
}

size_t strbuf_length(const strbuf_t str)
{
	return strnlen(str.s, str.size);
}

size_t strbuf_size(const strbuf_t str)
{
	return str.size;
}

int strbuf_equals(const strbuf_t lhs, const strview_t rhs)
{
	return strview_equals(strbuf_to_strview(lhs), rhs);
}

size_t strbuf_insert(strbuf_t str, size_t index, const strview_t sub)
{
	return str_insert(str.s, str.size, index, sub.s, sub.length);
}

size_t strbuf_delete(strbuf_t str, size_t index, size_t count)
{
	return str_delete(str.s, str.size, index, count);
}

size_t strbuf_set(strbuf_t dst, const strview_t src)
{
	return str_set(dst.s, dst.size, src);
}

size_t strbuf_setf(strbuf_t dst, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	int count = vsnprintf(dst.s, dst.size, format, ap);
	va_end(ap);

	return count;
}

size_t strbuf_append(strbuf_t dst, const strview_t src)
{
	return str_append(dst.s, strbuf_size(dst), src.s, src.length);
}

void strbuf_fill(strbuf_t dst, char c)
{
	memset(dst.s, c, dst.size);
}