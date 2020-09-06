#include "lodge_plugin_vfs.h"

#include "vfs.h"

struct lodge_ret lodge_plugin_vfs_new_inplace(struct vfs *vfs, struct lodge_plugins *plugins)
{
	vfs_new_inplace(vfs);
	return lodge_success();
}

struct lodge_plugin lodge_plugin_vfs()
{
	return (struct lodge_plugin) {
		.version = LODGE_PLUGIN_VERSION,
		.size = vfs_sizeof(),
		.name = strview_static("vfs"),
		.init = &lodge_plugin_vfs_new_inplace,
		.free = &vfs_free_inplace,
		.update = &vfs_update,
		.render = NULL,
	};
}
