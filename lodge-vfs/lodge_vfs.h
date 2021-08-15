//
// Virtual File System
//
#ifndef _LODGE_VFS_H
#define _LODGE_VFS_H

#include "strbuf.h"
#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

#define LODGE_VFS_FILENAME_MAX			4096
#define LODGE_VFS_FILES_MAX				1024
#define LODGE_VFS_MOUNT_PATH_MAX		LODGE_VFS_FILENAME_MAX
#define LODGE_VFS_MOUNT_POINT_MAX		LODGE_VFS_FILENAME_MAX
#define LODGE_VFS_MOUNTS_PER_POINT_MAX	32
#define LODGE_VFS_MOUNT_POINTS_MAX		32

struct lodge_vfs;

typedef void			(*lodge_vfs_func_t)(struct lodge_vfs *vfs, strview_t virtual_path, void *userdata);

void					lodge_vfs_new_inplace(struct lodge_vfs *vfs);
void					lodge_vfs_free_inplace(struct lodge_vfs *vfs);
size_t					lodge_vfs_sizeof();

void					lodge_vfs_update(struct lodge_vfs *vfs, float delta_time);

void					lodge_vfs_mount(struct lodge_vfs *vfs, strview_t mount_point, strview_t disk_path);
void					lodge_vfs_register_callback(struct lodge_vfs *vfs, strview_t virtual_path, lodge_vfs_func_t fn, void *userdata);
void					lodge_vfs_prune_callbacks(struct lodge_vfs *vfs, lodge_vfs_func_t fn, void* userdata);
bool					lodge_vfs_reload_file(struct lodge_vfs *vfs, strview_t virtual_path);

void*					lodge_vfs_read_file(struct lodge_vfs *vfs, strview_t virtual_path, size_t *out_num_bytes);
char*					lodge_vfs_read_text_file(struct lodge_vfs *vfs, strview_t virtual_path, size_t *out_num_bytes);

bool					lodge_vfs_resolve_disk_path(struct lodge_vfs *vfs, strview_t virtual_path, strbuf_t disk_path_out);

#endif
