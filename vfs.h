#ifndef _VFS_H
#define _VFS_H

#include <stdlib.h>

typedef void(*read_callback_t)(const char* filename, unsigned int size, void* data, void* userdata);

void	vfs_init(int argc, char **argv);
void	vfs_shutdown();
void	vfs_mount(const char* dir);
void	vfs_register_callback(const char* filename, read_callback_t fn, void* userdata);
void	vfs_register_callback_filter(const char* filter, read_callback_t fn, void* userdata);
void	vfs_run_callbacks();
#ifdef VFS_ENABLE_FILEWATCH
void	vfs_filewatch();
#else
#define vfs_filewatch(...)
#endif

void*	vfs_get_file(const char* filename, size_t* out_num_bytes);
void	vfs_free_memory(const char* filename);

int			vfs_file_count();
const char*	vfs_get_simple_name(const int index);

#endif //_VFS_H
