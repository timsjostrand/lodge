#ifndef _VFS_H
#define _VFS_H

#ifdef VFS_ENABLE_FILEWATCH
#define VFS_FILEWATCH vfs_filewatch();
#else
#define VFS_FILEWATCH
#endif

typedef void(*read_callback_t)(const char* filename, unsigned int size, void* data, void* userdata);

void	vfs_init();
void	vfs_shutdown();
void	vfs_mount(const char* dir);
void	vfs_register_callback(const char* filename, read_callback_t fn, void* userdata);
void	vfs_register_callback_filter(const char* filter, read_callback_t fn, void* userdata);
void	vfs_run_callbacks();
void	vfs_filewatch();

void*	vfs_get_file(const char* filename, unsigned int* out_num_bytes);
void	vfs_free_memory(const char* filename);

#endif //_VFS_H
