/**
 * Dynamic strings.
 *
 * Note that operations on handles that returns a handle MUST update the original handle!
 *
 * Usage:
 *		txt_t my_text = txt_new("hello!");
 *		printf("%s\n", my_text);
 *		// Output: "hello!"
 *
 *		my_text = txt_insert(my_text, 0, txt_new(I say: "));
 *		printf("%s\n", my_text);
 *		// Output: "I say: hello!"
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <string.h>
#include <assert.h>

#include "txt.h"
#include "str.h"

struct txt
{
	size_t	count;
	size_t	max_count;
};

static const size_t txt_header_size = sizeof(struct txt);

static struct txt* txt_from_handle(txt_t handle)
{
	return (struct txt *)(handle - txt_header_size);
}

static txt_t txt_to_handle(struct txt *txt)
{
	return (txt_t)((char *)txt + txt_header_size);
}

static void txt_fix_count(struct txt *txt)
{
	txt->count = strnlen(txt_to_handle(txt), txt->max_count);
}

static char* txt_tail(txt_t handle)
{
	return handle + txt_count(handle);
}

txt_t txt_new(const strview_t str)
{
	struct txt *txt = (struct txt *)malloc(txt_header_size + str.length + 1);

	txt->count = str.length;
	txt->max_count = str.length + 1;

	txt_t handle = txt_to_handle(txt);
	int excluded = str_set(handle, txt->max_count, str);
	assert(excluded == 0);

	return handle;
}

void txt_free(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	free(txt);
}

txt_t txt_reserve(txt_t handle, size_t new_size)
{
	struct txt *txt = txt_from_handle(handle);
	if(new_size < txt->max_count) {
		return handle;
	}
	txt->max_count = new_size;
	txt = (struct txt *)realloc(txt, txt_header_size + new_size);
	return txt_to_handle(txt);
}

txt_t txt_grow(txt_t handle, size_t fit)
{
	fit++; // Make room for \0
	struct txt *txt = txt_from_handle(handle);
	size_t new_max_count = txt->max_count;
	while(new_max_count < fit) {
		new_max_count *= 2;
	}
	if(new_max_count != txt->max_count) {
		handle = txt_reserve(handle, new_max_count);
	}
	return handle;
}

size_t txt_count(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	return txt->count;
}

txt_t txt_insert(txt_t handle, size_t index, const strview_t sub)
{
	handle = txt_grow(handle, txt_count(handle) + sub.length);
	struct txt *txt = txt_from_handle(handle);
	str_insert(handle, txt->max_count, index, sub.s, sub.length);
	txt->count += sub.length;
	return handle;
}

void txt_delete(txt_t handle, size_t index, size_t count)
{
	struct txt *txt = txt_from_handle(handle);
	if(!str_delete(handle, txt->max_count, index, count)) {
		assert(0);
	}
	txt_fix_count(txt); // TODO: smart check: if deleted at end of string, middle of string, ...
}

void txt_delete_from_tail(txt_t handle, size_t count)
{
	struct txt *txt = txt_from_handle(handle);
	count = min(txt->count, count);
	memset(txt_tail(handle) - count, '\0', count);
	txt->count -= count;
}

void txt_trim(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	txt->count = str_trim(handle, txt->count);
	assert(txt->count == strnlen(handle, txt->max_count));
}

int txt_begins_with(txt_t handle, const strview_t sub)
{
	struct txt *txt = txt_from_handle(handle);
	if(sub.length > txt->count) {
		return 0;
	} else {
		return memcmp(handle, sub.s, sub.length) == 0;
	}
}

int txt_ends_with(txt_t handle, const strview_t sub)
{
	struct txt *txt = txt_from_handle(handle);
	if(sub.length > txt->count) {
		return 0;
	} else {
		return memcmp(handle + txt->count - sub.length, sub.s, sub.length) == 0;
	}
}

#if 0
strbuf_t txt_to_strbuf(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	strbuf_t tmp = {
		.size = txt->max_count,
		.s = handle,
	};
	return tmp;
}
#endif

strview_t txt_to_strview(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	strview_t tmp = {
		.length = txt->count,
		.s = handle,
	};
	return tmp;
}