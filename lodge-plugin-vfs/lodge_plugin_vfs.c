#include "lodge_plugin_vfs.h"

#include "vfs.h"

struct lodge_ret lodge_plugin_vfs_new_inplace(struct vfs *vfs, struct lodge_plugins *plugins)
{
	if(vfs_new_inplace(vfs) == VFS_OK) {
		return lodge_success();
	} else {
		return lodge_error("Failed to init VFS");
	}
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
