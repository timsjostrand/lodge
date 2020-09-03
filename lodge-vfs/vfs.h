#ifndef _VFS_H
#define _VFS_H

#include "strview.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define VFS_OK 0
#define VFS_ERROR -1

#define VFS_MAX_FILENAME_LEN 256
#define VFS_MAX_NUM_FILES 1024
#define VFS_MOUNT_PATH_MAX 256

struct vfs;

typedef void			(*read_callback_t)(struct vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);

struct read_callback
{
	read_callback_t		fn;
	void				*userdata;
};

int						vfs_new_inplace(struct vfs *vfs);
void					vfs_free_inplace(struct vfs *vfs);
void					vfs_update(struct vfs *vfs, float delta_time);
size_t					vfs_sizeof();

void					vfs_mount(struct vfs *vfs, strview_t dir);
void					vfs_register_callback(struct vfs *vfs, strview_t filename, read_callback_t fn, void *userdata);
void					vfs_register_callback_filter(struct vfs *vfs, strview_t filter, read_callback_t fn, void *userdata);
void					vfs_run_callbacks(struct vfs *vfs);
void					vfs_prune_callbacks(struct vfs *vfs, read_callback_t fn, void* userdata);
int						vfs_reload_file(struct vfs *vfs, strview_t filename);

void*					vfs_get_file(struct vfs *vfs, strview_t filename, size_t *out_num_bytes);
void					vfs_free_memory(struct vfs *vfs, strview_t filename);

int						vfs_file_count(struct vfs *vfs);
strview_t				vfs_get_simple_name(struct vfs *vfs, const int index);
strview_t				vfs_get_absolute_path(struct vfs *vfs, strview_t filename);

#endif //_VFS_H
