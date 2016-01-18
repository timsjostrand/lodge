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
#include "log.h"
#define STB_DEFINE
#include "stb/stb.h"

typedef unsigned long DWORD;

#define vfs_error(...) errorf("VFS", __VA_ARGS__)

void vfs_init(const char *mount_path)
{
	for (int i = 0; i < MAX_NUM_FILES; i++)
	{
		vfs_global->file_table[i].name[0] = '\0';
		vfs_global->file_table[i].size = 0;
		vfs_global->file_table[i].data = 0;
		vfs_global->file_table[i].read_callbacks = 0;
	}

	if (mount_path != NULL) {
		vfs_mount(mount_path);
	}
}

void vfs_shutdown()
{
	for (int i = 0; i < MAX_NUM_FILES; i++)
	{
		stb_fclose(vfs_global->file_table[i].file, 0);
		if (vfs_global->file_table[i].data != NULL) {
			free(vfs_global->file_table[i].data);
		}
	}
}

void vfs_register_callback(const char* filename, read_callback_t fn, void* userdata)
{
	int added = 0;

	struct read_callback cbck;
	cbck.fn = fn;
	cbck.userdata = userdata;

	for (int i = 0; i < vfs_global->file_count; i++)
	{
		if (strcmp(filename, vfs_global->file_table[i].simplename) == 0)
		{
			stb_arr_push(vfs_global->file_table[i].read_callbacks, cbck);
			added = 1;
		}
	}

	if (!added)
	{
		vfs_global->file_table[vfs_global->file_count].data = 0;
		vfs_global->file_table[vfs_global->file_count].file = 0;
		vfs_global->file_table[vfs_global->file_count].lastChange = 0;
		strcpy(vfs_global->file_table[vfs_global->file_count].name, filename);
		strcpy(vfs_global->file_table[vfs_global->file_count].simplename, filename);
		vfs_global->file_table[vfs_global->file_count].size = 0;
		stb_arr_push(vfs_global->file_table[vfs_global->file_count].read_callbacks, cbck);
		vfs_global->file_count++;
	}
}

void vfs_register_callback_filter(const char* filter, read_callback_t fn, void* userdata)
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		if (strstr(vfs_global->file_table[i].name, filter) != 0)
		{
			struct read_callback cbck;
			cbck.fn = fn;
			cbck.userdata = userdata;
			stb_arr_push(vfs_global->file_table[i].read_callbacks, cbck);
		}
	}
}

static void vfs_reload(struct vfs_file *f, int force)
{
	time_t lastChange = stb_ftimestamp(f->name);
	if (f->lastChange != lastChange || force)
	{
		f->file = stb_fopen(f->name, "rb");

		if (f->file == 0)
		{
			return;
		}

		fseek(f->file, 0, SEEK_SET);

		free(f->data);
		f->lastChange = lastChange;

		// Hacky solution to make sure the OS is finished with the fseek call
		// How can this be solved better?
		f->size = 0;
		while (f->size == 0)
		{
			f->size = stb_filelen(f->file);
		}

		f->data = malloc(f->size);
		fread(f->data, 1, f->size, f->file);
		stb_fclose(f->file, 0);

		for (int j = 0, j_size = stb_arr_len(f->read_callbacks); j < j_size; j++)
		{
			read_callback_t cbck = f->read_callbacks[j].fn;
			cbck(f->simplename, f->size, f->data, f->read_callbacks[j].userdata);
		}
	}
}

static struct vfs_file* vfs_get_file_entry(const char *filename)
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		struct vfs_file *f = &vfs_global->file_table[i];

		if (strcmp(filename, f->simplename) == 0)
		{
			return f;
		}
	}

	return NULL;
}

int vfs_reload_file(const char *filename)
{
	struct vfs_file *f = vfs_get_file_entry(filename);

	if(f == NULL)
	{
		return VFS_ERROR;
	}

	vfs_reload(f, 1);
	return VFS_OK;
}

#ifdef VFS_ENABLE_FILEWATCH
void vfs_filewatch()
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		vfs_reload(&vfs_global->file_table[i], 0);
	}
}
#endif

void vfs_run_callbacks()
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		for (int j = 0, j_size = stb_arr_len(vfs_global->file_table[i].read_callbacks); j < j_size; j++)
		{
			read_callback_t cbck = vfs_global->file_table[i].read_callbacks[j].fn;
			cbck(vfs_global->file_table[i].simplename, vfs_global->file_table[i].size, vfs_global->file_table[i].data, vfs_global->file_table[i].read_callbacks[j].userdata);
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
		for (int j = 0; j < vfs_global->file_count; j++)
		{
			if (strcmp(vfs_global->file_table[j].simplename, new_file.simplename) == 0)
			{
				stb_fclose(vfs_global->file_table[j].file, 0);
				free(vfs_global->file_table[j].data);

				strcpy(vfs_global->file_table[j].name, new_file.name);
				vfs_global->file_table[j].file = stb_fopen(vfs_global->file_table[j].name, "rb");
				vfs_global->file_table[j].lastChange = stb_ftimestamp(vfs_global->file_table[j].name);
				vfs_global->file_table[j].size = stb_filelen(vfs_global->file_table[j].file);
				vfs_global->file_table[j].data = malloc(vfs_global->file_table[j].size);
				fread(vfs_global->file_table[j].data, 1, vfs_global->file_table[j].size, vfs_global->file_table[j].file);
				stb_fclose(vfs_global->file_table[i].file, 0);

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

			vfs_global->file_table[vfs_global->file_count] = new_file;
			vfs_global->file_count++;
		}
	}
}

void* vfs_get_file(const char* filename, size_t* out_num_bytes)
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		if (strcmp(filename, vfs_global->file_table[i].simplename) == 0 && vfs_global->file_table[i].data != 0)
		{
			*out_num_bytes = vfs_global->file_table[i].size;
			return vfs_global->file_table[i].data;
		}
	}

	return 0;
}

void vfs_free_memory(const char* filename)
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		if (strcmp(filename, vfs_global->file_table[i].simplename) == 0)
		{
			free(vfs_global->file_table[i].data);
			vfs_global->file_table[i].data = 0;
		}
	}
}

int vfs_file_count()
{
	return vfs_global->file_count;
}

const char* vfs_get_simple_name(const int index)
{
	return vfs_global->file_table[index].simplename;
}

const char* vfs_get_absolute_path(const char* filename)
{
	for (int i = 0; i < vfs_global->file_count; i++)
	{
		if (strcmp(filename, vfs_global->file_table[i].simplename) == 0 && vfs_global->file_table[i].data != 0)
		{
			return vfs_global->file_table[i].name;
		}
	}

	return 0;
}
