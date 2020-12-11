#include "lodge_plugin_fbx.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_res.h"

#include "fbx.h"
#include "fbx_asset.h"

#define USERDATA_FILES 0

static bool lodge_res_fbx_new_inplace(struct lodge_res *res, strview_t name, lodge_res_id_t id, void *fbx_asset_ptr, size_t size)
{
	struct fbx_asset *fbx_asset = (struct fbx_asset *)fbx_asset_ptr;

	struct lodge_res *files = lodge_res_get_userdata(res, USERDATA_FILES);
	ASSERT(files);

	const struct lodge_res_file *file = lodge_res_get_depend(files, name, (struct lodge_res_handle) {
		.resources = res,
		.id = id,
	});
	if(!file) {
		return false;
	}

	struct fbx *fbx = fbx_new(file->data, file->size);
	if(!fbx) {
		return false;
	}

	*fbx_asset = fbx_asset_make(fbx);

	fbx_free(fbx);

	return true;
}

static void lodge_res_fbx_free_inplace(struct lodge_res *res, strview_t name, lodge_res_id_t id, struct fbx_asset *fbx_asset)
{
	struct lodge_res *files = lodge_res_get_userdata(res, USERDATA_FILES);
	ASSERT(files);

	lodge_res_release_depend(files, name, (struct lodge_res_handle) {
		.resources = res,
		.id = id,
	});
	fbx_asset_reset(fbx_asset);
}

static struct lodge_ret lodge_plugin_fbx_init(struct lodge_res *fbx_res, struct lodge_plugins *plugins)
{
	struct lodge_res *files = lodge_plugins_depend(plugins, fbx_res, strview_static("files"));
	if(!files) {
		return lodge_error("FBX failed to find file res");
	}

	lodge_res_new_inplace(fbx_res, (struct lodge_res_desc) {
		.name = strview_static("fbx"),
		.size = sizeof(struct fbx_asset),
		.new_inplace = &lodge_res_fbx_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_res_fbx_free_inplace
	} );

	lodge_res_set_userdata(fbx_res, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_plugin_fbx_free(struct lodge_res *fbx_res)
{
	lodge_res_free_inplace(fbx_res);
}

struct lodge_plugin lodge_plugin_fbx()
{
	return (struct lodge_plugin) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_res_sizeof(),
		.name = strview_static("fbx"),
		.init = &lodge_plugin_fbx_init,
		.free = &lodge_plugin_fbx_free,
		.update = NULL,
		.render = NULL,
	};
}