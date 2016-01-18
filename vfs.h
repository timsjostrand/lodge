#ifndef _VFS_H
#define _VFS_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define VFS_OK 0
#define VFS_ERROR -1

#define MAX_FILENAME_LEN 256
#define MAX_NUM_FILES 256

typedef void(*read_callback_t)(const char* filename, unsigned int size, void* data, void* userdata);

struct read_callback
{
	read_callback_t fn;
	void* userdata;
};

struct vfs_file
{
	FILE* file;
	char name[MAX_FILENAME_LEN];
	char simplename[MAX_FILENAME_LEN];
	time_t lastChange;
	struct read_callback* read_callbacks;
	size_t size;
	void* data;
};

struct vfs
{
	struct vfs_file	file_table[MAX_NUM_FILES];
	int				file_count;
};

struct vfs* vfs_global;

void	vfs_init(const char *mount_path);
void	vfs_shutdown();
void	vfs_mount(const char* dir);
void	vfs_register_callback(const char* filename, read_callback_t fn, void* userdata);
void	vfs_register_callback_filter(const char* filter, read_callback_t fn, void* userdata);
void	vfs_run_callbacks();
int		vfs_reload_file(const char *filename);
#ifdef VFS_ENABLE_FILEWATCH
void	vfs_filewatch();
#else
#define vfs_filewatch(...)
#endif

void*	vfs_get_file(const char* filename, size_t* out_num_bytes);
void	vfs_free_memory(const char* filename);

int			vfs_file_count();
const char*	vfs_get_simple_name(const int index);
const char* vfs_get_absolute_path(const char* filename);

#endif //_VFS_H
