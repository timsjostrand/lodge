#ifndef _BLOB_CUR_H
#define _BLOB_CUR_H

#include <stddef.h>

struct blob_cur
{
	const char			*begin;
	const char			*it;
	const char * const	end;
};

struct blob_cur			blob_cur_make(const char* buf, size_t buf_size);
struct blob_cur			blob_cur_make_from_cur(const struct blob_cur *cur, size_t sub_size);
struct blob_cur			blob_cur_make_from_start(const struct blob_cur *cur, size_t offset);
int						blob_cur_contains(const struct blob_cur *cur, size_t offset);
int						blob_cur_can_read(const struct blob_cur *cur, size_t size);
size_t					blob_cur_remaining(const struct blob_cur *cur);
size_t					blob_cur_offset(const struct blob_cur *cur);
size_t					blob_cur_size(const struct blob_cur *cur);
int						blob_cur_is_empty(const struct blob_cur *cur);
int						blob_cur_read_type(char* dst, size_t dst_size, struct blob_cur *cur);
int						blob_cur_advance(struct blob_cur *dst, size_t offset);
int						blob_cur_mirror(struct blob_cur *dst, const struct blob_cur *src);
#define					blob_cur_read(dst, blob_cur) blob_cur_read_type((char*)(&dst), sizeof(dst), blob_cur)

#endif