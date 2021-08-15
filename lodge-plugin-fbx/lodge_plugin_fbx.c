#include "lodge_plugin_fbx.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_assets.h"

#include "fbx.h"
#include "fbx_asset.h"

#define USERDATA_FILES 0

static bool lodge_res_fbx_new_inplace(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, void *fbx_asset_ptr, size_t size)
{
	struct fbx_asset *fbx_asset = (struct fbx_asset *)fbx_asset_ptr;

	struct lodge_assets *files = lodge_assets_get_userdata(assets, USERDATA_FILES);
	ASSERT(files);

	const struct lodge_asset_file *file = lodge_assets_get_depend(files, name, (struct lodge_asset_handle) {
		.assets = assets,
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

static void lodge_res_fbx_free_inplace(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, struct fbx_asset *fbx_asset)
{
	struct lodge_assets *files = lodge_assets_get_userdata(assets, USERDATA_FILES);
	ASSERT(files);

	lodge_assets_release_depend(files, name, (struct lodge_asset_handle) {
		.assets = assets,
		.id = id,
	});
	fbx_asset_reset(fbx_asset);
}

static struct lodge_ret lodge_plugin_fbx_new_inplace(struct lodge_assets *fbx_assets, struct lodge_plugins *plugins)
{
	struct lodge_assets *files = lodge_plugins_depend(plugins, fbx_assets, strview_static("files"));
	if(!files) {
		return lodge_error("FBX failed to find file res");
	}

	lodge_assets_new_inplace(fbx_assets, (struct lodge_assets_desc) {
		.name = strview_static("fbx"),
		.size = sizeof(struct fbx_asset),
		.new_inplace = &lodge_res_fbx_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_res_fbx_free_inplace
	} );

	lodge_assets_set_userdata(fbx_assets, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_plugin_fbx_free_inplace(struct lodge_assets *fbx_res)
{
	lodge_assets_free_inplace(fbx_res);
}

struct lodge_plugin_desc lodge_plugin_fbx()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets_sizeof(),
		.name = strview_static("fbx"),
		.new_inplace = &lodge_plugin_fbx_new_inplace,
		.free_inplace = &lodge_plugin_fbx_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}