#ifndef _STR_H
#define _STR_H

#include <stdlib.h>

#ifndef HAVE_STRNLEN
size_t	strnlen(const char *s, size_t maxlen);
#endif

int		str_insert(char *s, size_t s_size, size_t index, const char *sub, size_t sub_len);
int		str_replace_into(char *s, size_t s_size, size_t index, const char *sub, size_t sub_len);
int		str_delete(char *s, size_t s_size, size_t index, size_t count);
int		str_append(char *s, size_t s_size, const char *sub, size_t sub_len);
char*	str_next_word(char *s, const size_t s_size, const size_t offset, const char *needles);
char*	str_prev_word(char *s, const size_t s_size, const size_t offset, const char *needles);
char*	str_search_reverse(char *s, const size_t s_size, const char needle,
				int needle_count);
char*	str_copy(const char *s, const size_t s_len, const size_t s_size);

int		str_parse_1f(const char *s, float *dst);

/**
 * Easily iterate over lines in a string.
 *
 * This macro will reserve the following variable names:
 *
 * "start"		Where in the text this line starts.
 * "end"		Where in the text this line ends.
 * "len"		The length of this line.
 * "i"			(For interal use) Character increment variable.
 * "next_start"	(For internal use) Where next line will start.
 *
 * Example:
 * @code
 * char *text = "foo\nbar\r\ntest";
 * size_t size = strlen(text) + 1;
 * foreach_line(text, size) {
 *	char *s = str_copy(text + start, len, len + 1);
 *	printf("%s\n", s);
 *	free(s);
 * }
 * @endcode
 *
 * The same example as above but without malloc:
 * @code
 * printf("%.*s\n", (int) len, text + start);
 * @endcode
 *
 * @param s		The string to iterate over.
 * @param size	The size of the string, including the \0 terminator.
 */
#define foreach_line(s, size) \
	for(size_t start = 0, next_start = 0, end = 0, len = 0, i = 0; \
			i <= size; \
			end = (i == size \
				|| s[i] == '\n' \
				|| s[i] == '\0') ? (i > 0 && s[i-1] == '\r' ? i-1 : i) : 0, \
			start = next_start, \
			len = (end != 0) ? end - start : 0, \
			next_start = (end != 0) ? i + 1 : next_start, \
			i++) \
	if(end != 0)

#endif
