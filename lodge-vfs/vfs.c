/**
* A simple virtual file system.
*
* Call vfs_init() on startup and vfs_shutdown() on shutdown.
* Use vfs_mount(const char* dir) to read all files in dir to memory.
* Use vfs_register_callback(const char* filename, read_callback_t fn) to read file
*
* Author: Johan Yngman <johan.yngman@gmail.com>
*         Tim Sjöstrand <tim.sjostrand@gmail.com>
*/

#include "vfs.h"

#include "strbuf.h"
#include "log.h"
#include "lodge_platform.h"
#include "lodge_filewatch.h"

#include "stb/stb.h"

struct vfs_file
{
	FILE*					file;
	char					name[VFS_MAX_FILENAME_LEN];
	char					simplename[VFS_MAX_FILENAME_LEN];
	struct read_callback*	read_callbacks;
	size_t					size;
	void					*data;
};

struct vfs
{
	struct vfs_file			file_table[VFS_MAX_NUM_FILES];
	size_t					file_count;
	struct lodge_filewatch	*filewatch;
};

static struct vfs_file* vfs_get_file_entry(struct vfs *vfs, strview_t filename)
{
	for (size_t i = 0; i < vfs->file_count; i++) {
		struct vfs_file *f = &vfs->file_table[i];

		if (strview_equals(filename, strview_wrap(f->simplename))) {
			return f;
		}
	}

	return NULL;
}

static void vfs_reload_from_filewatch(strview_t path, enum lodge_filewatch_reason reason, struct vfs *vfs)
{
	struct vfs_file *vfs_file = vfs_get_file_entry(vfs, path);
	ASSERT(vfs_file);
	if(!vfs_file) {
		return;
	}

	vfs_file->file = fopen(vfs_file->name, "rb");

	if(vfs_file->file == 0) {
		return;
	}

	fseek(vfs_file->file, 0, SEEK_SET);

	free(vfs_file->data);

	// Hacky solution to make sure the OS is finished with the fseek call
	// How can this be solved better?
	size_t last_size = -1;
	vfs_file->size = 0;
	while(vfs_file->size == 0 || last_size != vfs_file->size) {
		vfs_file->size = stb_filelen(vfs_file->file);
		last_size = vfs_file->size;
	}

	vfs_file->data = malloc(vfs_file->size);
	fread(vfs_file->data, 1, vfs_file->size, vfs_file->file);
	stb_fclose(vfs_file->file, 0);

	for(int j = 0, j_size = stb_arr_len(vfs_file->read_callbacks); j < j_size; j++) {
		read_callback_t cbck = vfs_file->read_callbacks[j].fn;
		cbck(vfs, strview_wrap(vfs_file->simplename), vfs_file->size, vfs_file->data, vfs_file->read_callbacks[j].userdata);
	}
}

void vfs_new_inplace(struct vfs *vfs)
{
	*vfs = (struct vfs) { 0 };

	vfs->filewatch = lodge_filewatch_new();
}

void vfs_free_inplace(struct vfs *vfs)
{
	lodge_filewatch_free(vfs->filewatch);

	for(int i = 0; i < VFS_MAX_NUM_FILES; i++) {
		stb_fclose(vfs->file_table[i].file, 0);
		if(vfs->file_table[i].data != NULL) {
			free(vfs->file_table[i].data);
		}
	}
}

void vfs_update(struct vfs *vfs, float delta_time)
{
	lodge_filewatch_update(vfs->filewatch, delta_time);
}

size_t vfs_sizeof()
{
	return sizeof(struct vfs);
}

void vfs_prune_callbacks(struct vfs *vfs, read_callback_t fn, void* userdata)
{
	int added = 0;

	struct read_callback cbck;
	cbck.fn = fn;
	cbck.userdata = userdata;

	for(size_t i = 0; i < vfs->file_count; i++) {
		struct vfs_file *file = &vfs->file_table[i];

		for(int j = 0; j < stb_arr_len(file->read_callbacks); j++) {
			struct read_callback *callback = &file->read_callbacks[j];

			if(callback->fn == fn && callback->userdata == userdata) {
				stb_arr_delete(file->read_callbacks, j);
				j--;
			}
		}
	}
}

void vfs_register_callback(struct vfs *vfs, strview_t filename, read_callback_t fn, void* userdata)
{
	int added = 0;

	struct read_callback cbck;
	cbck.fn = fn;
	cbck.userdata = userdata;

	for (size_t i = 0; i < vfs->file_count; i++) {
		if (strview_equals(filename, strview_wrap(vfs->file_table[i].simplename))) {
			stb_arr_push(vfs->file_table[i].read_callbacks, cbck);
			added = 1;
		}
	}

	if (!added) {
		struct vfs_file *new_file = &vfs->file_table[vfs->file_count++];

		new_file->data = 0;
		new_file->file = 0;
		strbuf_wrap_and(new_file->name, strbuf_set, filename);
		strbuf_wrap_and(new_file->simplename, strbuf_set, filename);
		new_file->size = 0;
		stb_arr_push(new_file->read_callbacks, cbck);
	}
}

bool vfs_reload_file(struct vfs *vfs, strview_t filename)
{
#if 0
	struct vfs_file *f = vfs_get_file_entry(vfs, filename);

	if(f == NULL) {
		return false;
	}

	vfs_reload(vfs, f, 1);
	return true;
#else
	ASSERT_NOT_IMPLEMENTED();
	return false;
#endif
}

void vfs_mount(struct vfs *vfs, strview_t dir)
{
	if (strview_empty(dir)) {
		ASSERT_FAIL("Failed to mount directory");
		return;
	}

	// FIXME(TS): use strbuf
	strbuf_t path;
	{
		char path_buf[VFS_MOUNT_PATH_MAX] = { 0 };
		path = strbuf_wrap(path_buf);
	}
	strbuf_set(path, dir);

	size_t pathlen = strbuf_length(path);

	if (pathlen == 0) {
		ASSERT_FAIL("Mount directory is of length 0");
		return;
	}

	if (path.s[pathlen - 1] == '\\' || path.s[pathlen - 1] == '/') {
		path.s[pathlen - 1] = '\0';
	}

	lodge_filewatch_add_dir(vfs->filewatch, strbuf_to_strview(path), true, &vfs_reload_from_filewatch, vfs);

	char** filenames = stb_readdir_recursive(path.s, NULL);

	if (filenames == NULL) {
		ASSERT_FAIL("Could not read directory");
		return;
	}

	int num_new_files = stb_arr_len(filenames);
	for (int i = 0; i < num_new_files; i++) {
		struct vfs_file new_file;
		strcpy(new_file.name, filenames[i]);
		strcpy(new_file.simplename, filenames[i] + strbuf_length(path) + 1);

		int replaced = 0;
		for (size_t j = 0; j < vfs->file_count; j++) {
			struct vfs_file *current_file = &vfs->file_table[j];

			if (strcmp(current_file->simplename, new_file.simplename) == 0) {
				stb_fclose(current_file->file, 0);
				free(current_file->data);

				strcpy(current_file->name, new_file.name);
				current_file->file = stb_fopen(current_file->name, "rb");
				current_file->size = stb_filelen(current_file->file);
				current_file->data = malloc(current_file->size);
				fread(current_file->data, 1, current_file->size, current_file->file);
				stb_fclose(current_file->file, 0); // NOTE(TS): was file_table[i] -- bug?

				replaced = 1;
				break;
			}
		}

		if (!replaced) {
			new_file.read_callbacks = 0;
			new_file.file = stb_fopen(new_file.name, "rb");
			new_file.size = stb_filelen(new_file.file);
			new_file.data = malloc(new_file.size);
			fread(new_file.data, 1, new_file.size, new_file.file);
			stb_fclose(new_file.file, 0);

			vfs->file_table[vfs->file_count] = new_file;
			vfs->file_count++;
		}
	}
}

void* vfs_get_file(struct vfs *vfs, strview_t filename, size_t* out_num_bytes)
{
	for (size_t i = 0; i < vfs->file_count; i++) {
		struct vfs_file *f = &vfs->file_table[i];

		if (strview_equals(filename, strview_wrap(f->simplename))
			&& f->data != 0) {
			*out_num_bytes = f->size;
			return f->data;
		}
	}

	return 0;
}

void vfs_free_memory(struct vfs *vfs, strview_t filename)
{
	for (size_t i = 0; i < vfs->file_count; i++) {
		struct vfs_file *f = &vfs->file_table[i];

		if (strview_equals(filename, strview_wrap(f->simplename))) {
			free(f->data);
			f->data = 0;
		}
	}
}

size_t vfs_file_count(struct vfs *vfs)
{
	return vfs->file_count;
}

strview_t vfs_get_simple_name(struct vfs *vfs, const int index)
{
	return strview_wrap(vfs->file_table[index].simplename);
}

strview_t vfs_get_absolute_path(struct vfs *vfs, strview_t filename)
{
	for (size_t i = 0; i < vfs->file_count; i++) {
		struct vfs_file *f = &vfs->file_table[i];
		
		if (strview_equals(filename, strview_wrap(f->simplename))
			&& f->data != 0) {
			return strview_wrap(f->name);
		}
	}

	return strview_null();
}
