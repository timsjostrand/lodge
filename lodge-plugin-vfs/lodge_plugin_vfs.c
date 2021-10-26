#include "lodge_plugin_vfs.h"

#include "lodge_vfs.h"
#include "lodge_argv.h"

struct lodge_ret lodge_plugin_vfs_new_inplace(struct lodge_vfs *vfs, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	lodge_vfs_new_inplace(vfs);

	if(lodge_argv_is_arg(args, strview("mount"))) {
		strview_t mount_dir = lodge_argv_get_str(args, strview("mount"), strview("assets/"));
		lodge_vfs_mount(vfs, strview("/"), mount_dir);
	}

	return lodge_success();
}

struct lodge_plugin_desc lodge_plugin_vfs()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_vfs_sizeof(),
		.name = strview("vfs"),
		.new_inplace = &lodge_plugin_vfs_new_inplace,
		.free_inplace = &lodge_vfs_free_inplace,
		.update = &lodge_vfs_update,
		.render = NULL,
	};
}
