/**
 * String helpers not found in <string.h> for static strings with a fixed
 * buffer size.
 *
 * When dealing with static strings, parameters named "x_size" is to be
 * interpreted as the "size of the buffer of x", that is the maximum possible
 * string length including the \0 terminator.
 *
 * Parameters named "x_len" should be interpreted to mean "the string length of
 * x", ie the number of characters in x excluding the \0 terminator.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "str.h"

/**
 * An implementation of the standard strnlen, if it is not already implemented.
 */
size_t strnlen(const char *s, size_t s_size)
{
	const char *e;
	size_t n;

	for(e = s, n = 0; *e && n < s_size; e++, n++);
	return n;
}

/**
 * Insert a character into a given position in a string, and move any adjacent
 * characters to accomodate the new substring.
 *
 * @param s			The string to delete from.
 * @param s_size	The size of the buffer holding s.
 * @param index		Delete starting from this index.
 * @param sub		The substring to insert.
 * @param sub_len	The length in characters of the substring.
 *
 * @return			The number of characters added to the string.
 *
 * @see				str_replace_into
 */
int str_insert(char *s, size_t s_size, size_t index, const char *sub, size_t sub_len)
{
	/* Sanity check: invalid arguments. */
	if(s_size == 0
			|| sub_len == 0
			|| s == NULL
			|| sub == NULL) {
		return 0;
	}
	/* Sanity check: out of bounds. */
	if(index >= s_size) {
		return 0;
	}
	/* Sanity check: out of bounds, operate on as many chars as possible. */
	if(index+sub_len+1 >= s_size) {
		sub_len = s_size - index;
	}
	/* Sanity check: inserting null terminator? */
	if(sub[sub_len - 1] == '\0') {
#ifdef DEBUG
		printf("WARN @ Str: str_insert() with subtext containing null terminator\n");
#endif
		if(sub_len == 0) {
			return 0;
		} else {
			sub_len --;
		}
	}
	/* Shift characters right. */
	memmove(s+index+sub_len, s+index, s_size-(index+sub_len));
	/* Insert new characters. */
	memcpy(s+index, sub, sub_len);
	/* Make sure string is null terminated. */
	s[s_size - 1] = '\0';
	/* Return number of characters added. */
	return sub_len;
}
/**
 * Implementation notes for str_insert():
 *
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 *  s = | A | B | C | D | E | 0 | 0 | 0 | 0 |
 *            |---|   |                   |
 *            |   |   ` index+sub_len+1   |
 *            |   ` index+sub_len         ` s_size
 *            ` index
 *
 *	str_insert(s, 9, 1, "xy", 2);
 *
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 *  s = | A | - | - | B | C | D | E | 0 | 0 |
 *
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 *  s = | A | x | y | B | C | D | E | 0 | 0 |
*/

/**
 * Insert a substring into a string, replacing any characters that were there
 * previously.
 *
 * @see str_insert
 */
int str_replace_into(char *s, size_t s_size, size_t index, const char *sub, size_t sub_len)
{
	/* Sanity check: invalid arguments. */
	if(s_size == 0
		|| sub_len == 0
		|| s == NULL
		|| sub == NULL) {
		return 0;
	}
	/* Sanity check: out of bounds. */
	if(index >= s_size) {
		return 0;
	}
	/* Sanity check: out of bounds, operate on as many chars as possible. */
	if(index + sub_len + 1 >= s_size) {
		sub_len = s_size - index;
	}
	/* Sanity check: inserting null terminator? */
	if(sub[sub_len - 1] == '\0') {
#ifdef DEBUG
		printf("WARN @ Str: str_replace_into() with subtext containing null terminator\n");
#endif
		if(sub_len == 0) {
			return 0;
		} else {
			sub_len--;
		}
	}
	/* Insert new characters. */
	memcpy(s + index, sub, sub_len);
	/* Make sure string is null terminated. */
	s[s_size - 1] = '\0';
	/* Return number of characters added. */
	return sub_len;
}

/**
 * Remove a set of characters from a string, and move the characters on the
 * right side of the deletion to fill the gap.
 * 
 * @param s			The string to delete from.
 * @param s_size	The size of the buffer holding s.
 * @param index		Delete from this index.
 * @param count		The number of characters to delete.
 *
 * @return			The number of characters removed from the string.
 */
int str_delete(char *s, size_t s_size, size_t index, size_t count)
{
	/* Sanity check: invalid arguments. */
	if(count < 1
			|| s_size == 0
			|| s == NULL) {
		return 0;
	}
	/* Sanity check: out of bounds. */
	if(index >= s_size - 1) {
		return 0;
	}
	/* Sanity check: out of bounds, operate on as many chars as possible. */
	if(index+count >= s_size) {
		count = s_size-index;
	}
	/* Shift chars to the left. */
	memmove(s+index, s+index+count, s_size-(index+count));
	/* Append null chars. */
	memset(s+s_size-count, '\0', count);
	return count;
}
/**
 * Implementation notes for str_delete():
 *
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 *	s = | A | B | C | D | E | 0 | 0 | 0 | 0 |
 *            |---|   |                   |
 *            |   |   ` index+count+1     ` s_size
 *            |   ` index+count
 *            ` index
 *
 *  str_delete(s, 9, 1, 2);
 *
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 *  s = | A | D | E | 0 | 0 | 0 | 0 | ? | ? |
 *
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 *  s = | A | D | E | 0 | 0 | 0 | 0 | 0 | 0 |
 *
 *
*/

int str_append(char *s, size_t s_size, const char *sub, size_t sub_len)
{
	size_t s_null = strnlen(s, s_size);
	/* Sanity check: out of bounds? */
	if(s_null + sub_len >= s_size) {
		sub_len = s_size - s_null;
	}
	memcpy(s+s_null, sub, sub_len);
	return sub_len;
}

/**
 * Find the next "word" in a string, where a word has a delimiter from the
 * 'needles' array.
 *
 * @return	The next word, or NULL if not found.
 */
char* str_next_word(char *s, const size_t s_size, const size_t offset, const char *needles)
{
	/* Sanity check: invalid argument. */
	if(s == NULL || offset > s_size) {
		return NULL;
	}
	int needle_count = strlen(needles);
	for(size_t i = offset; i < s_size; i++) {
		for(int n=0; n<needle_count; n++) {
			if(s[i] == needles[n] && s[i+1] != needles[n]) {
				return s + i;
			}
		}
	}
	return NULL;
}

/**
 * Find the prev "word" in a string, where a word has a delimiter from the
 * 'needles' array.
 *
 * @return	The prev word, or NULL if not found.
 */
char* str_prev_word(char *s, const size_t s_size, const size_t offset, const char *needles)
{
	/* Sanity check: invalid argument. */
	if(s == NULL || offset > s_size) {
		return NULL;
	}
	int needle_count = strlen(needles);
	for(size_t i = offset; i > 0; i--) {
		for(int n=0; n<needle_count; n++) {
			if(s[i] == needles[n] && s[i+1] != needles[n]) {
				return s + i;
			}
		}
	}
	return NULL;
}

/**
 * Searches from the back of the string after the char 'needle' the specified
 * amount of times and returns a pointer to the last occurance of 'needle'.
 *
 * If the search fails, NULL is returned instead.
 */
char* str_search_reverse(char *s, const size_t s_size, const char needle,
		int needle_count)
{
	char *tmp = s;
	for(int i=s_size; i>=0; i--) {
		if(needle_count <= 0) {
			break;
		}
		if(s[i] == needle) {
			tmp = &s[i];
			needle_count --;
		}
	}
	if(needle_count != 0) {
		return NULL;
	} else {
		return tmp;
	}
}

/**
 * Allocates new memory and copies a string to it. Equivalent to the
 * non-standard strndup().
 */
char* str_copy(const char *s, const size_t s_len, const size_t s_size)
{
	char *tmp = (char *) malloc(s_size);
	if(tmp == NULL) {
		return NULL;
	}
	memcpy(tmp, s, s_len * sizeof(char));
	tmp[s_len] = '\0';
	return tmp;
}

/**
 * Parses a float from a string.
 *
 * @return -1 on error, 0 on success.
 */
int str_parse_1f(const char *s, float *dst)
{
	char *end;
	float f = strtof(s, &end);
	if(end == NULL || s == end || (*end) != '\0') {
		return -1;
	}
	(*dst) = f;
	return 0;
}

void str_print_hex(const char *s)
{
	while(*s) printf("%02x ", (unsigned int) *s++);
}
