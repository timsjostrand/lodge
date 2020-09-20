#ifndef _LODGE_FILEWATCH_H
#define _LODGE_FILEWATCH_H

#include "strview.h"

#include <stdbool.h>

// TODO(TS):
//		- Add _add_file() varaint of _add_dir -- this can use _add_dir internally but will register a separate callback for the specific file
//		- Add _remove_dir(), _remove_file() API

enum lodge_filewatch_reason
{
	LODGE_FILEWATCH_REASON_FILE_CREATED,
	LODGE_FILEWATCH_REASON_FILE_DELETED,
	LODGE_FILEWATCH_REASON_FILE_MODIFIED,
	LODGE_FILEWATCH_REASON_DIR_CREATED,
	LODGE_FILEWATCH_REASON_DIR_DELETED,
	LODGE_FILEWATCH_REASON_DIR_MODIFIED,
	LODGE_FILEWATCH_REASON_MAX
};

typedef void				(*lodge_filewatch_func_t)(strview_t path, enum lodge_filewatch_reason reason, void *userdata);

struct lodge_filewatch;

struct lodge_filewatch*		lodge_filewatch_new();
void						lodge_filewatch_free(struct lodge_filewatch *filewatch);

void						lodge_filewatch_update(struct lodge_filewatch *filewatch, float dt);

void						lodge_filewatch_add_dir(struct lodge_filewatch *filewatch, strview_t dir_path, bool recursive, lodge_filewatch_func_t func, void *func_userdata);
//void						lodge_filewatch_add_file(struct lodge_filewatch *filewatch, strview_t file_path, lodge_filewatch_func_t func, void *func_userdata);

#endif