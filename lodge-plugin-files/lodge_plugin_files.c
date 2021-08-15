#include "lodge_plugin_files.h"

#include "lodge_plugins.h"
#include "lodge_res.h"

#include "lodge_vfs.h"

#define USERDATA_VFS 0

static void vfs_callback_wrapper(struct lodge_vfs *vfs, strview_t filename, struct lodge_res *userdata)
{
	lodge_res_reload(userdata, filename);
	//ASSERT(ret);
}

static bool lodge_res_files_new_inplace(struct lodge_res *res, strview_t name, lodge_res_id_t id, struct lodge_res_file *out, size_t size)
{
	struct lodge_vfs *vfs = lodge_res_get_userdata(res, USERDATA_VFS);
	ASSERT(vfs);
	if(!vfs) {
		return false;
	}

	size_t vfs_size = 0;
	char* vfs_data = lodge_vfs_read_file(vfs, name, &vfs_size);
	if(!vfs_data) {
		return false;
	}

	lodge_vfs_register_callback(vfs, name, &vfs_callback_wrapper, res);

	*out = (struct lodge_res_file) {
		.name = name,
		.data = vfs_data,
		.size = vfs_size,
	};

	return true;
}

static int lodge_res_files_free_inplace(struct lodge_res *res, strview_t name, lodge_res_id_t id, struct lodge_res_file *data)
{
	if(data->data) {
		free(data->data);
	}

	struct lodge_vfs *vfs = lodge_res_get_userdata(res, USERDATA_VFS);
	ASSERT(vfs);
	if(!vfs) {
		return false;
	}

	// FIXME(TS): should just remove one callback
	lodge_vfs_prune_callbacks(vfs, &vfs_callback_wrapper, res);
	return true;
}

static struct lodge_ret lodge_plugin_files_new_inplace(struct lodge_res *files, struct lodge_plugins *plugins)
{
	struct lodge_vfs *vfs = lodge_plugins_depend(plugins, files, strview_static("vfs"));
	if(!vfs) {
		return lodge_error("Files failed to find VFS");
	}

	lodge_res_new_inplace(files, (struct lodge_res_desc) {
		.name = strview_static("files"),
		.size = sizeof(struct lodge_res_file),
		.new_inplace = &lodge_res_files_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_res_files_free_inplace
	});

	lodge_res_set_userdata(files, USERDATA_VFS, vfs);

	return lodge_success();
}

static void lodge_plugin_files_free_inplace(struct lodge_res *res)
{
	lodge_res_free_inplace(res);
}

struct lodge_plugin_desc lodge_plugin_files()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_res_sizeof(),
		.name = strview_static("files"),
		.new_inplace = &lodge_plugin_files_new_inplace,
		.free_inplace = &lodge_plugin_files_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}
