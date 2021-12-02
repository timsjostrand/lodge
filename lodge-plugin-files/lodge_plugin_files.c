#include "lodge_plugin_files.h"

#include "dynbuf.h"

#include "lodge_plugins.h"
#include "lodge_assets2.h"
#include "lodge_vfs.h"

enum files_userdata
{
	USERDATA_VFS,
};

enum lodge_plugin_idx
{
	PLUGIN_IDX_VFS,
	PLUGIN_IDX_MAX,
};

struct lodge_assets_callback
{
	struct lodge_assets2			*assets;
	lodge_file_filter_func_t		filter;
};

struct lodge_file_discovery
{
	size_t							count;
	size_t							capacity;
	struct lodge_assets_callback	*elements;

	struct lodge_vfs				*vfs;
};

//
// TODO(TS): Sort out file reloading and asset discovery:
// 
//		* When a mount is added in VFS, check for new asset discoveries
//		* When a mount is removed in VFS, check for lost assets (can we can prune those not loaded in memory?)
//		* Chained asset discoveries: files => images => textures. Textures should accept any image as an asset.
//

static void lodge_file_discovery_scan_entry(struct lodge_file_discovery *file_discovery, struct lodge_vfs *vfs, strview_t path, struct lodge_assets_callback *entry)
{
	struct lodge_vfs_iterate_dynbuf vfs_ret;
	dynbuf_new_inplace(dynbuf(vfs_ret), 128);

	bool ret = lodge_vfs_iterate(vfs, path, strview("*"), &vfs_ret);
	ASSERT(ret);
	LODGE_UNUSED(ret);

	for(size_t i = 0, count = vfs_ret.count; i < count; i++) {
		if(!vfs_ret.elements[i].dir) {
			strview_t name = strview_wrap(vfs_ret.elements[i].name);
			if(entry->filter(name)) {
				lodge_assets2_register(entry->assets, strview_substring_from_start(name, 1));
			}
		}
	}

	dynbuf_free_inplace(dynbuf(vfs_ret));
}

static void lodge_file_discovery_scan(struct lodge_file_discovery *file_discovery, strview_t path, struct lodge_vfs *vfs)
{
	for(size_t i = 0, count = file_discovery->count; i < count; i++) {
		lodge_file_discovery_scan_entry(file_discovery, vfs, path, &file_discovery->elements[i]);
	}
}

static void lodge_file_discovery_on_vfs_modified(struct lodge_vfs *vfs, strview_t virtual_path, struct lodge_file_discovery *file_discovery)
{
	lodge_file_discovery_scan(file_discovery, virtual_path, vfs);
}

static void lodge_file_discovery_new_inplace(struct lodge_file_discovery *file_discovery, struct lodge_vfs *vfs)
{
	file_discovery->vfs = vfs;
	dynbuf_new_inplace(dynbuf_ptr(file_discovery), 8);
	lodge_vfs_add_global_callback(vfs, &lodge_file_discovery_on_vfs_modified, file_discovery);
}

static void lodge_file_discovery_free_inplace(struct lodge_file_discovery *file_discovery)
{
	lodge_vfs_remove_global_callback(file_discovery->vfs, &lodge_file_discovery_on_vfs_modified, file_discovery);
	dynbuf_free_inplace(dynbuf_ptr(file_discovery));
}

static struct lodge_file_discovery* lodge_file_discovery_get(struct lodge_assets2 *files)
{
	return (struct lodge_file_discovery *)(((char *)files) + lodge_assets2_sizeof());
}

static void lodge_asset_file_on_modified(struct lodge_vfs *vfs, strview_t filename, struct lodge_assets2 *files)
{
	lodge_asset_t asset = lodge_assets2_find_by_name(files, filename);
	if(asset) {
		lodge_assets2_invalidate(files, asset);
	}
}

static bool lodge_asset_file_new_inplace(struct lodge_assets2 *files, strview_t name, lodge_asset_t asset, struct lodge_asset_file *out)
{
	struct lodge_vfs *vfs = lodge_assets2_get_userdata(files, USERDATA_VFS);
	ASSERT_OR(vfs) {
		return false;
	}

	size_t vfs_size = 0;
	char* vfs_data = lodge_vfs_read_file(vfs, name, &vfs_size);
	if(!vfs_data) {
		return false;
	}

	lodge_vfs_register_callback(vfs, name, &lodge_asset_file_on_modified, files);

	*out = (struct lodge_asset_file) {
		.name = name,
		.data = vfs_data,
		.size = vfs_size,
		.vfs_callback = true,
	};

	return true;
}

static int lodge_asset_file_free_inplace(struct lodge_assets2 *files, strview_t name, lodge_asset_t asset, struct lodge_asset_file *data)
{
	if(data->data) {
		free(data->data);
		data->data = NULL;
	}

	struct lodge_vfs *vfs = lodge_assets2_get_userdata(files, USERDATA_VFS);
	ASSERT_OR(vfs) {
		return false;
	}

	if(data->vfs_callback) {
		const bool callback_removed = lodge_vfs_remove_callback(vfs, name, &lodge_asset_file_on_modified, files);
		ASSERT(callback_removed);
		LODGE_UNUSED(callback_removed);
	}

	return true;
}

static void lodge_files_on_mount_added(struct lodge_vfs *vfs, strview_t virtual_path, struct lodge_assets2 *files)
{
	struct lodge_file_discovery *file_discovery = lodge_file_discovery_get(files);

	for(size_t i = 0, count = file_discovery->count; i < count; i++) {
		struct lodge_assets_callback *callback = &file_discovery->elements[i];
		lodge_file_discovery_scan_entry(file_discovery, vfs, virtual_path, callback);
	}
}

static struct lodge_ret lodge_files_new_inplace(struct lodge_assets2 *files, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	{
		struct lodge_vfs *vfs = dependencies[PLUGIN_IDX_VFS];

		lodge_assets2_set_userdata(files, USERDATA_VFS, vfs);

		struct lodge_file_discovery *discovery = lodge_file_discovery_get(files);
		lodge_file_discovery_new_inplace(discovery, vfs);

		lodge_vfs_add_on_mount_added_func(vfs, &lodge_files_on_mount_added, files);
	}

	lodge_assets2_new_inplace(files, &(struct lodge_assets2_desc) {
		.name = strview("files"),
		.size = sizeof(struct lodge_asset_file),
		.new_inplace = &lodge_asset_file_new_inplace,
		.free_inplace = &lodge_asset_file_free_inplace,
	});

	return lodge_success();
}

static void lodge_files_free_inplace(struct lodge_assets2 *files)
{
	{
		struct lodge_vfs *vfs = lodge_assets2_get_userdata(files, USERDATA_VFS);
		lodge_vfs_remove_on_mount_added_func(vfs, &lodge_files_on_mount_added, files);
	}
	{
		struct lodge_file_discovery *file_discovery = lodge_file_discovery_get(files);
		lodge_file_discovery_free_inplace(file_discovery);
	}
	lodge_assets2_free_inplace(files);
}

void lodge_plugin_files_add_file_discovery(struct lodge_assets2 *files, struct lodge_assets2 *populate, lodge_file_filter_func_t filter)
{
	struct lodge_file_discovery *file_discovery = lodge_file_discovery_get(files);

	struct lodge_assets_callback *callback = dynbuf_append(dynbuf_ptr(file_discovery), &(struct lodge_assets_callback) {
		.assets = populate,
		.filter = filter,
	}, sizeof(struct lodge_assets_callback));

	struct lodge_vfs *vfs = lodge_assets2_get_userdata(files, USERDATA_VFS);

	lodge_file_discovery_scan_entry(file_discovery, vfs, strview("/"), callback);
}

LODGE_PLUGIN_IMPL(lodge_plugin_files)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof() + sizeof(struct lodge_file_discovery),
		.name = strview("files"),
		.new_inplace = &lodge_files_new_inplace,
		.free_inplace = &lodge_files_free_inplace,
		.update = NULL,
		.render = NULL,
		.dependencies ={
			.count = PLUGIN_IDX_MAX,
			.elements = {
				[PLUGIN_IDX_VFS] = {
					.name = strview("vfs"),
				}
			}
		}
	};
}