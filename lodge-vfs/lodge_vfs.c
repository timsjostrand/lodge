/**
 * Author: Johan Yngman <johan.yngman@gmail.com>
 *         Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include "lodge_vfs.h"

#include "strbuf.h"
#include "membuf.h"
#include "dynbuf.h"
#include "lodge_platform.h"
#include "lodge_filewatch.h"

#include "stb/stb.h"

struct lodge_vfs_mount
{
	char							point[LODGE_VFS_MOUNT_POINT_MAX];
	char							path[LODGE_VFS_MOUNT_PATH_MAX];
};

struct lodge_vfs_func
{
	lodge_vfs_func_t				fn;
	void							*userdata;
};

struct lodge_vfs_file_funcs
{
	char							virtual_path[LODGE_VFS_FILENAME_MAX];
	uint32_t						virtual_path_hash;
	struct lodge_vfs_func*			funcs;
};

struct lodge_vfs
{
	struct lodge_vfs_mount			mounts[LODGE_VFS_MOUNT_POINTS_MAX];
	size_t							mounts_count;

	struct lodge_vfs_file_funcs		funcs[LODGE_VFS_FILES_MAX];
	size_t							funcs_count;

	struct lodge_filewatch			*filewatch;
};

static struct lodge_vfs_file_funcs* lodge_vfs_get_func_entry(struct lodge_vfs *vfs, strview_t virtual_path)
{
	uint32_t virtual_path_hash = strview_calc_hash(virtual_path);

	for (size_t i = 0; i < vfs->funcs_count; i++) {
		struct lodge_vfs_file_funcs *func_entry = &vfs->funcs[i];

#if 0
		if (strview_equals(virtual_path, strview_wrap(func_entry->virtual_path))) {
			return func_entry;
		}
#else
		if(func_entry->virtual_path_hash == virtual_path_hash) {
			return func_entry;
		}
#endif
	}
	return NULL;
}

static void lodge_vfs_broadcast_file_funcs(struct lodge_vfs *vfs, struct lodge_vfs_file_funcs *file_funcs)
{
	for(int i = 0, count = stb_arr_len(file_funcs->funcs); i < count; i++) {
		struct lodge_vfs_func *func = &file_funcs->funcs[i];
		func->fn(vfs, strview_wrap(file_funcs->virtual_path), func->userdata);
	}
}

static void lodge_vfs_filewatch_event(strview_t path, enum lodge_filewatch_reason reason, struct lodge_vfs *vfs)
{
	struct lodge_vfs_file_funcs *file_funcs = lodge_vfs_get_func_entry(vfs, path);
	if(!file_funcs) {
		//ASSERT_FAIL("No callbacks for this file");
		return;
	}

	// TODO(TS): we are currently firing reload events for files reloaded in mounts with lower precedence;
	// `path` should be a disk_path, and should reverse resolve into a virtual_path
	// It currently works, because the listener needs to resolve in _read_file() anyways

	lodge_vfs_broadcast_file_funcs(vfs, file_funcs);
}

void lodge_vfs_new_inplace(struct lodge_vfs *vfs)
{
	memset(vfs, 0, sizeof(struct lodge_vfs));
	vfs->filewatch = lodge_filewatch_new();
}

void lodge_vfs_free_inplace(struct lodge_vfs *vfs)
{
	lodge_filewatch_free(vfs->filewatch);

	for(int i = 0; i < vfs->funcs_count; i++) {
		stb_arr_free(vfs->funcs[i].funcs);
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

void lodge_vfs_prune_callbacks(struct lodge_vfs *vfs, lodge_vfs_func_t fn, void* userdata)
{
	int added = 0;

	struct lodge_vfs_func cbck;
	cbck.fn = fn;
	cbck.userdata = userdata;

	for(size_t i = 0; i < vfs->funcs_count; i++) {
		struct lodge_vfs_file_funcs *func_entry = &vfs->funcs[i];

		for(int j = 0; j < stb_arr_len(func_entry->funcs); j++) {
			struct lodge_vfs_func *callback = &func_entry->funcs[j];

			if(callback->fn == fn && callback->userdata == userdata) {
				stb_arr_delete(func_entry->funcs, j);
				j--;
			}
		}
	}
}

void lodge_vfs_register_callback(struct lodge_vfs *vfs, strview_t virtual_path, lodge_vfs_func_t fn, void* userdata)
{
	struct lodge_vfs_file_funcs *func_entry = lodge_vfs_get_func_entry(vfs, virtual_path);

	struct lodge_vfs_func cbck = {
		.fn = fn,
		.userdata = userdata,
	};

	if(!func_entry) {
		func_entry = membuf_append_no_init(membuf_wrap(vfs->funcs), &vfs->funcs_count);
		func_entry->funcs = NULL;
		strbuf_set(strbuf_wrap(func_entry->virtual_path), virtual_path);
		func_entry->virtual_path_hash = strview_calc_hash(virtual_path);
	}

	ASSERT(func_entry);
	stb_arr_push(func_entry->funcs, cbck);
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

void lodge_vfs_mount(struct lodge_vfs *vfs, strview_t mount_point, strview_t dir)
{
	if (strview_empty(dir)) {
		ASSERT_FAIL("Failed to mount directory");
		return;
	}

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

	lodge_filewatch_add_dir(vfs->filewatch, strbuf_to_strview(path), true, &lodge_vfs_filewatch_event, vfs);

	{
		struct lodge_vfs_mount new_mount;
		strbuf_set(strbuf_wrap(new_mount.point), mount_point);
		strbuf_set(strbuf_wrap(new_mount.path), dir);
		membuf_append(membuf_wrap(vfs->mounts), &vfs->mounts_count, &new_mount, sizeof(struct lodge_vfs_mount));
	}

	// TODO(TS): we need to scan new mount and fire modify callbacks for all new files we have callbacks for,
	// since this mount will take precedence

	char** filenames = stb_readdir_recursive(path.s, NULL);
	ASSERT(filenames);
	for(int i=0, count = stb_arr_len(filenames); i < count; i++) {
		const size_t filename_len = strlen(filenames[i]);
		strview_t virtual_filename = strview_make(filenames[i] + strbuf_length(path) + 1, filename_len - strbuf_length(path));

		struct lodge_vfs_file_funcs* file_funcs = lodge_vfs_get_func_entry(vfs, virtual_filename);
		if(file_funcs) {
			lodge_vfs_broadcast_file_funcs(vfs, file_funcs);
		}
	}
}

void* lodge_vfs_read_file(struct lodge_vfs *vfs, strview_t virtual_path, size_t* out_num_bytes)
{
	char disk_path[LODGE_VFS_FILENAME_MAX];

	if(lodge_vfs_resolve_disk_path(vfs, virtual_path, strbuf_wrap(disk_path))) {
		FILE *file = stb_fopen(disk_path, "rb");
		*out_num_bytes = stb_filelen(file);
		char *data = malloc(*out_num_bytes);
		fread(data, 1, *out_num_bytes, file);
		stb_fclose(file, 0);
		return data;
	}

	return NULL;
}

char* lodge_vfs_read_text_file(struct lodge_vfs *vfs, strview_t virtual_path, size_t *out_num_bytes)
{
	char *data = lodge_vfs_read_file(vfs, virtual_path, out_num_bytes);
	if(!data) {
		return NULL;
	}

	if(data[*out_num_bytes - 1] != '\0') {
		*out_num_bytes += 1;
		char *new_data = realloc(data, *out_num_bytes);
		if(new_data) {
			new_data[*out_num_bytes - 1] = '\0';
			return new_data;
		}
	}

	return data;
}

bool lodge_vfs_resolve_disk_path(struct lodge_vfs *vfs, strview_t virtual_path, strbuf_t disk_path_out)
{
	if(virtual_path.length == 0) {
		return false;
	}

	// Make sure `virtual_path` begins with `/`
	char virtual_path_tmp[4096];
	strbuf_t virtual_path_tmp_buf = strbuf_wrap(virtual_path_tmp);
	if(virtual_path.s[0] != '/') {
		strbuf_setf(virtual_path_tmp_buf, "/" STRVIEW_PRINTF_FMT, STRVIEW_PRINTF_ARG(virtual_path));
		virtual_path = strbuf_to_strview(virtual_path_tmp_buf);
	}

	// TODO(TS): add resolve cache, sync from file watcher

	for(size_t i=0, count=vfs->mounts_count; i<count; i++) {
		const size_t index = count - 1 - i;
		struct lodge_vfs_mount *mount = &vfs->mounts[index];

		strbuf_t mount_point_buf = strbuf_wrap(mount->point);
		const size_t mount_point_len = strbuf_length(mount_point_buf);

		if(strview_begins_with(virtual_path, strbuf_to_strview(mount_point_buf))) {
			strview_t substring = strview_substring_from_start(virtual_path, mount_point_len);
			strbuf_setf(disk_path_out, "%s/" STRVIEW_PRINTF_FMT, mount->path, STRVIEW_PRINTF_ARG(substring));

			if(stb_fexists(disk_path_out.s)) {
				return true;
			}
		}
	}

	return false;
}

bool lodge_vfs_iterate(struct lodge_vfs *vfs, strview_t path, strview_t mask, struct lodge_vfs_iterate_dynbuf *out)
{
	ASSERT_OR(vfs) {
		return false;
	}
	ASSERT_NULL_TERMINATED(path);
	ASSERT_NULL_TERMINATED(mask);

	struct lodge_vfs_iterate_dynbuf ret = { 0 };

	for(size_t i = 0, count = vfs->mounts_count; i < count; i++) {
		struct lodge_vfs_mount *mount = &vfs->mounts[i];
		const size_t mount_path_length = strbuf_length(strbuf_wrap(mount->path));

		if(strview_begins_with(strbuf_to_strview(strbuf_wrap(mount->point)), path)) {
			// Files
			{
				char **mount_files = stb_readdir_recursive(mount->path, (char*)mask.s);
				size_t mount_files_count = stb_arr_len(mount_files);
				for(size_t i = 0; i < mount_files_count; i++) {
					const char *mount_file = mount_files[i];

					struct lodge_vfs_entry *entry = dynbuf_append_no_init(dynbuf_wrap(out));
					entry->dir = false;
					strbuf_setf(strbuf_wrap(entry->name), "%s%s", mount->point, mount_file + mount_path_length + 1);
				}
				stb_arr_free(mount_files);
			}

#if 0
			// Dirs
			{
				char **mount_dirs = stb_readdir_subdirs(mount->path);
				size_t mount_dirs_count = stb_arr_len(mount_dirs);
				for(size_t i = 0; i < mount_dirs_count; i++) {
					const char *mount_dir = mount_dirs[i];

					struct lodge_vfs_entry *entry = dynbuf_append_no_init(dynbuf_wrap(out));
					entry->dir = true;
					strbuf_setf(strbuf_wrap(entry->name), "%s%s", mount->point, mount_dir + mount_path_length + 1);
				}
				stb_arr_free(mount_dirs);
			}
#endif
		}
	}

	return true;
}