//
// Virtual File System
//
#ifndef _LODGE_VFS_H
#define _LODGE_VFS_H

#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

#define LODGE_VFS_FILENAME_MAX		4096
#define LODGE_VFS_FILES_MAX			1024
#define LODGE_VFS_MOUNT_PATH_MAX	LODGE_VFS_FILENAME_MAX

struct lodge_vfs;

typedef void			(*read_callback_t)(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);

struct read_callback
{
	read_callback_t		fn;
	void				*userdata;
};

void					lodge_vfs_new_inplace(struct lodge_vfs *vfs);
void					lodge_vfs_free_inplace(struct lodge_vfs *vfs);
size_t					lodge_vfs_sizeof();

void					lodge_vfs_update(struct lodge_vfs *vfs, float delta_time);

void					lodge_vfs_mount(struct lodge_vfs *vfs, strview_t dir);
void					lodge_vfs_register_callback(struct lodge_vfs *vfs, strview_t filename, read_callback_t fn, void *userdata);
void					lodge_vfs_prune_callbacks(struct lodge_vfs *vfs, read_callback_t fn, void* userdata);
bool					lodge_vfs_reload_file(struct lodge_vfs *vfs, strview_t filename);

void*					lodge_vfs_get_file(struct lodge_vfs *vfs, strview_t filename, size_t *out_num_bytes);
void					lodge_vfs_free_memory(struct lodge_vfs *vfs, strview_t filename);

size_t					lodge_vfs_file_count(struct lodge_vfs *vfs);
strview_t				lodge_vfs_get_simple_name(struct lodge_vfs *vfs, const int index);
strview_t				lodge_vfs_get_absolute_path(struct lodge_vfs *vfs, strview_t filename);

#endif
