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
char*	str_copy(const char *s, const size_t s_size);

int		str_parse_1f(const char *s, float *dst);

#endif
