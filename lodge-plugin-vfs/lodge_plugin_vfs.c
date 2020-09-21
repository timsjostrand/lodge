#include "lodge_plugin_vfs.h"

#include "lodge_vfs.h"

struct lodge_ret lodge_plugin_vfs_new_inplace(struct lodge_vfs *vfs, struct lodge_plugins *plugins)
{
	lodge_vfs_new_inplace(vfs);
	return lodge_success();
}

struct lodge_plugin lodge_plugin_vfs()
{
	return (struct lodge_plugin) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_vfs_sizeof(),
		.name = strview_static("vfs"),
		.init = &lodge_plugin_vfs_new_inplace,
		.free = &lodge_vfs_free_inplace,
		.update = &lodge_vfs_update,
		.render = NULL,
	};
}
