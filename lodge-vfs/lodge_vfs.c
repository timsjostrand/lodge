/**
 * Author: Johan Yngman <johan.yngman@gmail.com>
 *         Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

//
// TODO(TS):
//
//		- Multiple mount points: /, /assets, etc
//		- Handle reloads from overloaded mount point
//
//

#include "lodge_vfs.h"

#include "strbuf.h"
#include "lodge_platform.h"
#include "lodge_filewatch.h"

#include "stb/stb.h"

struct lodge_vfs_file
{
	char					name[LODGE_VFS_FILENAME_MAX];
	char					simplename[LODGE_VFS_FILENAME_MAX];
	struct read_callback*	read_callbacks;
	size_t					size;
	void					*data;
};

struct lodge_vfs
{
	struct lodge_vfs_file	file_table[LODGE_VFS_FILES_MAX];
	size_t					file_count;
	struct lodge_filewatch	*filewatch;
};

static struct lodge_vfs_file* vfs_get_file_entry(struct lodge_vfs *vfs, strview_t filename)
{
	for (size_t i = 0; i < vfs->file_count; i++) {
		struct lodge_vfs_file *f = &vfs->file_table[i];

		if (strview_equals(filename, strview_wrap(f->simplename))) {
			return f;
		}
	}

	return NULL;
}

static void vfs_reload_from_filewatch(strview_t path, enum lodge_filewatch_reason reason, struct lodge_vfs *vfs)
{
	struct lodge_vfs_file *vfs_file = vfs_get_file_entry(vfs, path);
	ASSERT(vfs_file);
	if(!vfs_file) {
		ASSERT_FAIL("File path not found in VFS");
		return;
	}

	FILE *file = fopen(vfs_file->name, "rb");
	if(!file) {
		ASSERT_FAIL("Failed to open file");
		return;
	}

	fseek(file, 0, SEEK_SET);

	free(vfs_file->data);

	// Hacky solution to make sure the OS is finished with the fseek call
	// How can this be solved better?
	size_t last_size = -1;
	vfs_file->size = 0;
	while(vfs_file->size == 0 || last_size != vfs_file->size) {
		vfs_file->size = stb_filelen(file);
		last_size = vfs_file->size;
	}

	vfs_file->data = malloc(vfs_file->size);
	fread(vfs_file->data, 1, vfs_file->size, file);
	stb_fclose(file, 0);

	for(int j = 0, j_size = stb_arr_len(vfs_file->read_callbacks); j < j_size; j++) {
		read_callback_t func = vfs_file->read_callbacks[j].fn;
		func(vfs, strview_wrap(vfs_file->simplename), vfs_file->size, vfs_file->data, vfs_file->read_callbacks[j].userdata);
	}
}

void lodge_vfs_new_inplace(struct lodge_vfs *vfs)
{
	memset(vfs, 0, sizeof(struct lodge_vfs));
	vfs->filewatch = lodge_filewatch_new();
}

void lodge_vfs_free_inplace(struct lodge_vfs *vfs)
{
	lodge_filewatch_free(vfs->filewatch);

	for(int i = 0; i < LODGE_VFS_FILES_MAX; i++) {
		if(vfs->file_table[i].data != NULL) {
			free(vfs->file_table[i].data);
		}
	}
}

void lodge_vfs_update(struct lodge_vfs *vfs, float delta_time)
{
	lodge_filewatch_update(vfs->filewatch, delta_time);
}

size_t lodge_vfs_sizeof()
{
	return sizeof(struct lodge_vfs);
}

void lodge_vfs_prune_callbacks(struct lodge_vfs *vfs, read_callback_t fn, void* userdata)
{
	int added = 0;

	struct read_callback cbck;
	cbck.fn = fn;
	cbck.userdata = userdata;

	for(size_t i = 0; i < vfs->file_count; i++) {
		struct lodge_vfs_file *file = &vfs->file_table[i];

		for(int j = 0; j < stb_arr_len(file->read_callbacks); j++) {
			struct read_callback *callback = &file->read_callbacks[j];

			if(callback->fn == fn && callback->userdata == userdata) {
				stb_arr_delete(file->read_callbacks, j);
				j--;
			}
		}
	}
}

void lodge_vfs_register_callback(struct lodge_vfs *vfs, strview_t filename, read_callback_t fn, void* userdata)
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
		struct lodge_vfs_file *new_file = &vfs->file_table[vfs->file_count++];

		new_file->data = 0;
		strbuf_wrap_and(new_file->name, strbuf_set, filename);
		strbuf_wrap_and(new_file->simplename, strbuf_set, filename);
		new_file->size = 0;
		stb_arr_push(new_file->read_callbacks, cbck);
	}
}

bool lodge_vfs_reload_file(struct lodge_vfs *vfs, strview_t filename)
{
#if 0
	struct lodge_vfs_file *f = vfs_get_file_entry(vfs, filename);

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

void lodge_vfs_mount(struct lodge_vfs *vfs, strview_t dir)
{
	if (strview_empty(dir)) {
		ASSERT_FAIL("Failed to mount directory");
		return;
	}

	// FIXME(TS): use strbuf
	strbuf_t path;
	{
		char path_buf[LODGE_VFS_MOUNT_PATH_MAX] = { 0 };
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
		struct lodge_vfs_file new_file;
		strcpy(new_file.name, filenames[i]);
		strcpy(new_file.simplename, filenames[i] + strbuf_length(path) + 1);

		int replaced = 0;
		for (size_t j = 0; j < vfs->file_count; j++) {
			struct lodge_vfs_file *current_file = &vfs->file_table[j];

			if (strcmp(current_file->simplename, new_file.simplename) == 0) {
				free(current_file->data);

				strcpy(current_file->name, new_file.name);

				{
					FILE *file = stb_fopen(current_file->name, "rb");
					current_file->size = stb_filelen(file);
					current_file->data = malloc(current_file->size);
					fread(current_file->data, 1, current_file->size, file);
					stb_fclose(file, 0);
				}

				replaced = 1;
				break;
			}
		}

		if (!replaced) {
			new_file.read_callbacks = 0;

			{
				FILE *file = stb_fopen(new_file.name, "rb");
				new_file.size = stb_filelen(file);
				new_file.data = malloc(new_file.size);
				fread(new_file.data, 1, new_file.size, file);
				stb_fclose(file, 0);
			}

			vfs->file_table[vfs->file_count] = new_file;
			vfs->file_count++;
		}
	}
}

void* lodge_vfs_get_file(struct lodge_vfs *vfs, strview_t filename, size_t* out_num_bytes)
{
	for(size_t i = 0; i < vfs->file_count; i++) {
		struct lodge_vfs_file *f = &vfs->file_table[i];

		if(strview_equals(filename, strview_wrap(f->simplename))
			&& f->data != 0) {
			*out_num_bytes = f->size;
			return f->data;
		}
	}

	return NULL;
}

void lodge_vfs_free_memory(struct lodge_vfs *vfs, strview_t filename)
{
	for(size_t i = 0; i < vfs->file_count; i++) {
		struct lodge_vfs_file *f = &vfs->file_table[i];

		if(strview_equals(filename, strview_wrap(f->simplename))) {
			free(f->data);
			f->data = 0;
		}
	}
}

size_t lodge_vfs_file_count(struct lodge_vfs *vfs)
{
	return vfs->file_count;
}

strview_t lodge_vfs_get_simple_name(struct lodge_vfs *vfs, const int index)
{
	return strview_wrap(vfs->file_table[index].simplename);
}

strview_t lodge_vfs_get_absolute_path(struct lodge_vfs *vfs, strview_t filename)
{
	for (size_t i = 0; i < vfs->file_count; i++) {
		struct lodge_vfs_file *f = &vfs->file_table[i];
		
		if (strview_equals(filename, strview_wrap(f->simplename))
			&& f->data != 0) {
			return strview_wrap(f->name);
		}
	}

	return strview_null();
}
