#ifndef _BLOB_H
#define _BLOB_H

#include <stddef.h>

enum blob_error
{
	BLOB_OK,
	BLOB_OPEN_ERROR,
	BLOB_WRITE_ERROR
};

struct blob;

struct blob*			blob_new_from_file(const char *filename);
void					blob_free(struct blob *blob);

const char*				blob_data(struct blob *blob);
size_t					blob_size(struct blob *blob);

enum blob_error			blob_write_to_file(struct blob *blob, const char *filename);

#endif
