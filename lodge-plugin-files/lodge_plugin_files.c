#include "lodge_plugin_files.h"

#include "lodge_plugins.h"
#include "lodge_assets.h"

#include "lodge_vfs.h"

#define USERDATA_VFS 0

static void vfs_callback_wrapper(struct lodge_vfs *vfs, strview_t filename, struct lodge_assets *userdata)
{
	lodge_assets_reload(userdata, filename);
	//ASSERT(ret);
}

static bool lodge_assets_files_new_inplace(struct lodge_assets *files, strview_t name, lodge_asset_id_t id, struct lodge_asset_file *out, size_t size)
{
	struct lodge_vfs *vfs = lodge_assets_get_userdata(files, USERDATA_VFS);
	ASSERT(vfs);
	if(!vfs) {
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

static int lodge_assets_files_free_inplace(struct lodge_assets *files, strview_t name, lodge_asset_id_t id, struct lodge_asset_file *data)
{
	if(data->data) {
		free(data->data);
	}

	struct lodge_vfs *vfs = lodge_assets_get_userdata(files, USERDATA_VFS);
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

static struct lodge_ret lodge_plugin_files_new_inplace(struct lodge_assets *files, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	struct lodge_vfs *vfs = lodge_plugins_depend(plugins, files, strview_static("vfs"));
	if(!vfs) {
		return lodge_error("Files failed to find VFS");
	}

	lodge_assets_new_inplace(files, (struct lodge_assets_desc) {
		.name = strview_static("files"),
		.size = sizeof(struct lodge_asset_file),
		.new_inplace = &lodge_assets_files_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_assets_files_free_inplace
	});

	lodge_assets_set_userdata(files, USERDATA_VFS, vfs);

	return lodge_success();
}

static void lodge_plugin_files_free_inplace(struct lodge_assets *res)
{
	lodge_assets_free_inplace(res);
}

struct lodge_plugin_desc lodge_plugin_files()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets_sizeof(),
		.name = strview_static("files"),
		.new_inplace = &lodge_plugin_files_new_inplace,
		.free_inplace = &lodge_plugin_files_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}
