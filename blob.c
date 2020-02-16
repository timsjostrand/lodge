#include "blob.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lodge_assert.h"

struct blob
{
	char*	buf;
	size_t	buf_size;
};

struct blob* blob_new_from_file(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if(!fp) {
		return NULL;
	}

	if(fseek(fp, 0L, SEEK_END) == -1) {
		ASSERT_FAIL("Failed to seek file");
		fclose(fp);
		return NULL;
	}
	size_t size_guess = ftell(fp);
	if(fseek(fp, 0L, SEEK_SET) == -1) {
		ASSERT_FAIL("Failed to guess file size");
		fclose(fp);
		return NULL;
	}

	struct blob *blob = (struct blob*)malloc(sizeof(struct blob));
	if(!blob) {
		ASSERT_FAIL("Failed to allocate blob");
		fclose(fp);
		return NULL;
	}

	blob->buf = (char *)malloc(size_guess);
	if(!blob->buf) {
		ASSERT_FAIL("Failed to allocate blob buffer");
		fclose(fp);
		return NULL;
	}

	size_t size_read = fread(blob->buf, 1, size_guess, fp);
	blob->buf_size = size_read;
	if(size_read != size_guess) {
		blob_free(blob);
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	return blob;
}

void blob_free(struct blob *blob)
{
	free(blob->buf);
	blob->buf = NULL;
	blob->buf_size = 0;
}

const char* blob_data(struct blob *blob)
{
	return blob->buf;
}

size_t blob_size(struct blob *blob)
{
	return blob->buf_size;
}

enum blob_error blob_write_to_file(struct blob *blob, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if(!fp) {
		return BLOB_OPEN_ERROR;
	}

	size_t size_written = fwrite(blob->buf, 1, blob->buf_size, fp);
	if(size_written != blob->buf_size) {
		fclose(fp);
		return BLOB_WRITE_ERROR;
	}

	fclose(fp);
	return BLOB_OK;
}



struct blob_cur blob_cur_make(const char* buf, size_t buf_size)
{
	struct blob_cur tmp = {
		.begin = buf,
		.it = buf,
		.end = buf + buf_size
	};
	return tmp;
}

struct blob_cur blob_cur_make_from_cur(const struct blob_cur *cur, size_t sub_size)
{
	ASSERT(blob_cur_can_read(cur, sub_size));

	struct blob_cur tmp = {
		.begin = cur->begin,
		.it = cur->it,
		.end = min(cur->it + sub_size, cur->end)
	};

	return tmp;
}

struct blob_cur blob_cur_make_from_start(const struct blob_cur *cur, size_t offset)
{
	ASSERT(blob_cur_contains(cur, offset));

	struct blob_cur tmp = {
		.begin = cur->begin,
		.it = cur->it,
		.end = min(cur->begin + offset, cur->end)
	};

	return tmp;
}

size_t blob_cur_size(const struct blob_cur *cur)
{
	return cur->end - cur->begin;
}

size_t blob_cur_remaining(const struct blob_cur *cur)
{
	return cur->end - cur->it;
}

int blob_cur_can_read(const struct blob_cur *cur, size_t size)
{
	return (cur->it + size) <= cur->end;
}

int blob_cur_contains(const struct blob_cur *cur, size_t offset)
{
	return (cur->begin + offset) <= cur->end;
}

int blob_cur_is_empty(const struct blob_cur *cur)
{
	return cur->it >= cur->end;
}

int blob_cur_read_type(char* dst, size_t dst_size, struct blob_cur *cur)
{
	if(!blob_cur_can_read(cur, dst_size)) {
		ASSERT_FAIL("Data cursor underrun");
		return 0;
	}
	memcpy(dst, cur->it, dst_size);
	cur->it += dst_size;
	return 1;
}

size_t blob_cur_offset(const struct blob_cur *cur)
{
	return cur->it - cur->begin;
}

int blob_cur_advance(struct blob_cur *dst, size_t offset)
{
	if(!blob_cur_contains(dst, (size_t)(dst->it - dst->begin) + offset)) {
		return 0;
	}
	dst->it += offset;
	return 1;
}

int blob_cur_mirror(struct blob_cur *dst, const struct blob_cur *src)
{
	if(dst->begin != src->begin) {
		return 0;
	}
	if(!blob_cur_contains(dst, blob_cur_offset(src))) {
		return 0;
	}
	dst->it = src->it;
	return 1;
}
