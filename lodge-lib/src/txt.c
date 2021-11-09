/**
 * Dynamic strings.
 *
 * Note that operations on handles that returns a handle MUST update the original handle!
 *
 * Usage:
 *		txt_t my_text = txt_new(strview_static("hello!"));
 *		printf("%s\n", my_text);
 *		// Output: "hello!"
 *
 *		my_text = txt_insert(my_text, 0, strview_static("I say: "));
 *		printf("%s\n", my_text);
 *		// Output: "I say: hello!"
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include "lodge_assert.h"
#include "lodge_platform.h"

#include "txt.h"
#include "str.h"
#include "math4.h"

#include <string.h>

struct txt
{
	size_t	length;
	size_t	size;
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

static void txt_fix_length(struct txt *txt)
{
	txt->length = strnlen(txt_to_handle(txt), txt->size);
}

static char* txt_tail(txt_t handle)
{
	return handle + txt_length(handle);
}

#if 0
static strbuf_t txt_to_strbuf(txt_t handle, struct txt *txt)
{
	return strbuf_make(handle, txt->size);
}
#endif

txt_t txt_new(const strview_t str)
{
	size_t txt_size = str.length + 1;
	struct txt *txt = (struct txt *)malloc(txt_header_size + txt_size);

	txt->length = str.length;
	txt->size = txt_size;

	txt_t handle = txt_to_handle(txt);
	int excluded = str_set(handle, txt_size, str);
	ASSERT(excluded == 0);
	LODGE_UNUSED(excluded);

	return handle;
}

void txt_free(txt_t handle)
{
	if(!handle) { return; } 
	struct txt *txt = txt_from_handle(handle);
	free(txt);
}

txt_t txt_reserve(txt_t handle, size_t new_size)
{
	struct txt *txt = txt_from_handle(handle);
	if(new_size < txt->size) {
		return handle;
	}
	txt->size = new_size;
	txt = (struct txt *)realloc(txt, txt_header_size + new_size);
	return txt_to_handle(txt);
}

txt_t txt_grow(txt_t handle, size_t fit)
{
	fit++; // Make room for \0
	struct txt *txt = txt_from_handle(handle);
	size_t new_size = txt->size;
	while(new_size < fit) {
		new_size *= 2;
	}
	if(new_size != txt->size) {
		handle = txt_reserve(handle, new_size);
	}
	return handle;
}

size_t txt_length(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	return txt->length;
}

txt_t txt_insert(txt_t handle, size_t index, const strview_t sub)
{
	handle = txt_grow(handle, txt_length(handle) + sub.length);
	struct txt *txt = txt_from_handle(handle);
	str_insert(handle, txt->size, index, sub.s, sub.length);
	txt->length += sub.length;
	return handle;
}

void txt_delete(txt_t handle, size_t index, size_t count)
{
	struct txt *txt = txt_from_handle(handle);
	if(!str_delete(handle, txt->size, index, count)) {
		ASSERT(0);
	}
	txt_fix_length(txt); // TODO: smart check: if deleted at end of string, middle of string, ...
}

void txt_delete_from_tail(txt_t handle, size_t count)
{
	struct txt *txt = txt_from_handle(handle);
	count = min(txt->length, count);
	memset(txt_tail(handle) - count, '\0', count);
	txt->length -= count;
}

void txt_clear(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	*handle = '\0';
	txt->length = 0;
}

txt_t txt_set(txt_t txt, strview_t str)
{
	txt_clear(txt);
	return txt_insert(txt, 0, str);
}

void txt_trim(txt_t handle)
{
	struct txt *txt = txt_from_handle(handle);
	txt->length = str_trim(handle, txt->length);
	ASSERT(txt->length == strnlen(handle, txt->size));
}

int txt_begins_with(txt_t handle, const strview_t sub)
{
	struct txt *txt = txt_from_handle(handle);
	if(sub.length > txt->length) {
		return 0;
	} else {
		return memcmp(handle, sub.s, sub.length) == 0;
	}
}

int txt_ends_with(txt_t handle, const strview_t sub)
{
	struct txt *txt = txt_from_handle(handle);
	if(sub.length > txt->length) {
		return 0;
	} else {
		return memcmp(handle + txt->length - sub.length, sub.s, sub.length) == 0;
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
		.length = txt->length,
		.s = handle,
	};
	return tmp;
}
