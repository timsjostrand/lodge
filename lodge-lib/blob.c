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
	free(blob);
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
