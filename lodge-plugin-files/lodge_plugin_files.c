#include "lodge_plugin_files.h"

#include "dynbuf.h"

#include "lodge_plugins.h"
#include "lodge_assets2.h"
#include "lodge_vfs.h"

enum files_userdata
{
	USERDATA_VFS,
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
};

static void lodge_file_discovery_new_inplace(struct lodge_file_discovery *file_discovery)
{
	dynbuf_new_inplace(dynbuf_ptr(file_discovery), 8);
}

static struct lodge_file_discovery* lodge_file_discovery_get(struct lodge_assets2 *files)
{
	return (struct lodge_file_discovery *)(((char *)files) + lodge_assets2_sizeof());
}

static void vfs_callback_wrapper(struct lodge_vfs *vfs, strview_t filename, struct lodge_assets2 *files)
{
	lodge_asset_t asset = lodge_assets2_find_by_name(files, filename);
	if(asset) {
		lodge_assets2_invalidate(files, asset);
	}
}

static bool lodge_files_new_inplace(struct lodge_assets2 *files, strview_t name, lodge_asset_t asset, struct lodge_asset_file *out)
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

	lodge_vfs_register_callback(vfs, name, &vfs_callback_wrapper, files);

	*out = (struct lodge_asset_file) {
		.name = name,
		.data = vfs_data,
		.size = vfs_size,
		.vfs_callback = true,
	};

	return true;
}

static int lodge_files_free_inplace(struct lodge_assets2 *files, strview_t name, lodge_asset_t asset, struct lodge_asset_file *data)
{
	if(data->data) {
		free(data->data);
	}

	struct lodge_vfs *vfs = lodge_assets2_get_userdata(files, USERDATA_VFS);
	ASSERT(vfs);
	if(!vfs) {
		return false;
	}

	if(data->vfs_callback) {
		const bool callback_removed = lodge_vfs_remove_callback(vfs, name, &vfs_callback_wrapper, files);
		ASSERT(callback_removed);
	}

	return true;
}

#if 0
static void foo(struct lodge_vfs *vfs, strview_t virtual_path, struct lodge_file_discovery *file_discovery)
{
	file_discovery->assets
}
#endif

static struct lodge_ret lodge_plugin_files_new_inplace(struct lodge_assets2 *files, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	struct lodge_vfs *vfs = lodge_plugins_depend(plugins, files, strview("vfs"));
	if(!vfs) {
		return lodge_error("Files failed to find VFS");
	}

	lodge_assets2_new_inplace(files, &(struct lodge_assets2_desc) {
		.name = strview("files"),
		.size = sizeof(struct lodge_asset_file),
		.new_inplace = &lodge_files_new_inplace,
		.free_inplace = &lodge_files_free_inplace,
	});

	lodge_assets2_set_userdata(files, USERDATA_VFS, vfs);

	{
		struct lodge_file_discovery *discovery = lodge_file_discovery_get(files);
		lodge_file_discovery_new_inplace(discovery);
		//lodge_vfs_add_global_callback(vfs, )

	}

	return lodge_success();
}

static void lodge_plugin_files_free_inplace(struct lodge_assets2 *files)
{
	//struct lodge_file_discovery *file_discovery = lodge_file_discovery_get(files);
	//lodge_file_discovery_free_inplace(file_discovery);
	lodge_assets2_free_inplace(files);
}

struct lodge_plugin_desc lodge_plugin_files()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof() + sizeof(struct lodge_file_discovery),
		.name = strview("files"),
		.new_inplace = &lodge_plugin_files_new_inplace,
		.free_inplace = &lodge_plugin_files_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

void lodge_plugin_files_add_file_discovery(struct lodge_assets2 *files, struct lodge_assets2 *populate, lodge_file_filter_func_t filter)
{
	struct lodge_file_discovery *file_discovery = lodge_file_discovery_get(files);

	dynbuf_append(dynbuf_ptr(file_discovery), &(struct lodge_assets_callback) {
		.assets = populate,
		.filter = filter,
	}, sizeof(struct lodge_assets_callback));

	struct lodge_vfs *vfs = lodge_assets2_get_userdata(files, USERDATA_VFS);

	struct lodge_vfs_iterate_dynbuf vfs_ret;
	dynbuf_new_inplace(dynbuf(vfs_ret), 128);

	bool ret = lodge_vfs_iterate(vfs, strview("/"), strview("*"), &vfs_ret);
	ASSERT(ret);

	for(size_t i = 0, count = vfs_ret.count; i < count; i++) {
		if(!vfs_ret.elements[i].dir) {
			strview_t name = strview_wrap(vfs_ret.elements[i].name);
			if(filter(name)) {
				lodge_assets2_register(populate, strview_substring_from_start(name, 1));
			}
		}
	}

	dynbuf_free_inplace(dynbuf(vfs_ret));
}
