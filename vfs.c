/**
* A simple virtual file system.
*
* Call vfs_init() on startup and vfs_shutdown() on shutdown.
* Use vfs_mount(const char* dir) to read all files in dir to memory.
* Use vfs_register_callback(const char* filename, read_callback_t fn) to read file
*
*	 Compile with VFS_ENABLE_FILEWATCH to enable filewatching
*
* Author: Johan Yngman <johan.yngman@gmail.com>
*/

#include "vfs.h"

typedef unsigned long DWORD;

#define STB_DEFINE
#include <stb/stb.h>

#define MAX_FILENAME_LEN 256
#define MAX_NUM_FILES 256

#define vfs_error(...) fprintf(stderr, "ERROR @ VFS: " __VA_ARGS__)

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

struct vfs_file file_table[MAX_NUM_FILES];
static int file_count = 0;

void vfs_init()
{
	for (int i = 0; i < MAX_NUM_FILES; i++)
	{
		file_table[i].name[0] = '\0';
		file_table[i].size = 0;
		file_table[i].data = 0;
		file_table[i].read_callbacks = 0;
	}
}

void vfs_shutdown()
{
	for (int i = 0; i < MAX_NUM_FILES; i++)
	{
		stb_fclose(file_table[i].file, 0);
		if (file_table[i].data != NULL) {
			free(file_table[i].data);
		}
	}
}

void vfs_register_callback(const char* filename, read_callback_t fn, void* userdata)
{
	int added = 0;

	struct read_callback cbck;
	cbck.fn = fn;
	cbck.userdata = userdata;

	for (int i = 0; i < file_count; i++)
	{
		if (strcmp(filename, file_table[i].simplename) == 0)
		{
			stb_arr_push(file_table[i].read_callbacks, cbck);
			added = 1;
		}
	}

	if (!added)
	{
		file_table[file_count].data = 0;
		file_table[file_count].file = 0;
		file_table[file_count].lastChange = 0;
		strcpy(file_table[file_count].name, filename);
		strcpy(file_table[file_count].simplename, filename);
		file_table[file_count].size = 0;
		stb_arr_push(file_table[file_count].read_callbacks, cbck);
		file_count++;
	}
}

void vfs_register_callback_filter(const char* filter, read_callback_t fn, void* userdata)
{
	for (int i = 0; i < file_count; i++)
	{
		if (strstr(file_table[i].name, filter) != 0)
		{
			struct read_callback cbck;
			cbck.fn = fn;
			cbck.userdata = userdata;
			stb_arr_push(file_table[i].read_callbacks, cbck);
		}
	}
}

#ifdef VFS_ENABLE_FILEWATCH
void vfs_filewatch()
{
	for (int i = 0; i < file_count; i++)
	{
		time_t lastChange = stb_ftimestamp(file_table[i].name);
		if (file_table[i].lastChange != lastChange)
		{
			file_table[i].file = stb_fopen(file_table[i].name, "rb");

			if (file_table[i].file == 0)
			{
				continue;
			}

			fseek(file_table[i].file, 0, SEEK_SET);

			free(file_table[i].data);
			file_table[i].lastChange = lastChange;

			// Hacky solution to make sure the OS is finished with the fseek call
			// How can this be solved better?
			file_table[i].size = 0;
			while (file_table[i].size == 0)
			{
				file_table[i].size = stb_filelen(file_table[i].file);
			}

			file_table[i].data = malloc(file_table[i].size);
			fread(file_table[i].data, 1, file_table[i].size, file_table[i].file);
			stb_fclose(file_table[i].file, 0);

			for (int j = 0, j_size = stb_arr_len(file_table[i].read_callbacks); j < j_size; j++)
			{
				read_callback_t cbck = file_table[i].read_callbacks[j].fn;
				cbck(file_table[i].simplename, file_table[i].size, file_table[i].data, file_table[i].read_callbacks[j].userdata);
			}
		}
	}
}
#endif

void vfs_run_callbacks()
{
	for (int i = 0; i < file_count; i++)
	{
		for (int j = 0, j_size = stb_arr_len(file_table[i].read_callbacks); j < j_size; j++)
		{
			read_callback_t cbck = file_table[i].read_callbacks[j].fn;
			cbck(file_table[i].simplename, file_table[i].size, file_table[i].data, file_table[i].read_callbacks[j].userdata);
		}
	}
}

void vfs_mount(const char* dir)
{
	if (dir == NULL)
	{
		vfs_error("Invalid mount argument\n");
		return;
	}

	char** filenames = stb_readdir_recursive(dir, NULL);

	if (filenames == NULL) 
	{ 
		vfs_error("Could not read directory: %s\n", dir);
		return;
	}

	int num_new_files = stb_arr_len(filenames);
	for (int i = 0; i < num_new_files; i++)
	{
		struct vfs_file new_file;
		strcpy(new_file.name, filenames[i]);
		strcpy(new_file.simplename, filenames[i] + strlen(dir) + 1);

		int replaced = 0;
		for (int j = 0; j < file_count; j++)
		{
			if (strcmp(file_table[j].simplename, new_file.simplename) == 0)
			{
				stb_fclose(file_table[j].file, 0);
				free(file_table[j].data);

				strcpy(file_table[j].name, new_file.name);
				file_table[j].file = stb_fopen(file_table[j].name, "rb");
				file_table[j].lastChange = stb_ftimestamp(file_table[j].name);
				file_table[j].size = stb_filelen(file_table[j].file);
				file_table[j].data = malloc(file_table[j].size);
				fread(file_table[j].data, 1, file_table[j].size, file_table[j].file);
				stb_fclose(file_table[i].file, 0);

				replaced = 1;
				break;
			}
		}

		if (!replaced)
		{
			new_file.read_callbacks = 0;
			new_file.file = stb_fopen(new_file.name, "rb");
			new_file.lastChange = stb_ftimestamp(new_file.name);
			new_file.size = stb_filelen(new_file.file);
			new_file.data = malloc(new_file.size);
			fread(new_file.data, 1, new_file.size, new_file.file);
			stb_fclose(new_file.file, 0);

			file_table[file_count] = new_file;
			file_count++;
		}
	}
}

void* vfs_get_file(const char* filename, size_t* out_num_bytes)
{
	for (int i = 0; i < file_count; i++)
	{
		if (strcmp(filename, file_table[i].simplename) == 0 && file_table[i].data != 0)
		{
			*out_num_bytes = file_table[i].size;
			return file_table[i].data;
		}
	}

	return 0;
}

void vfs_free_memory(const char* filename)
{
	for (int i = 0; i < file_count; i++)
	{
		if (strcmp(filename, file_table[i].simplename) == 0)
		{
			free(file_table[i].data);
			file_table[i].data = 0;
		}
	}
}
