#include "lodge_plugin_fbx.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_assets2.h"
#include "lodge_type_asset.h"

#include "fbx.h"
#include "fbx_asset.h"

enum lodge_assets_fbx_userdata
{
	USERDATA_FILES,
	USERDATA_ASSET_TYPE,
};

enum lodge_plugin_idx
{
	PLUGIN_IDX_FILES,
	PLUGIN_IDX_MAX,
};

static bool lodge_asset_fbx_new_inplace(struct lodge_assets2 *fbx_assets, strview_t name, lodge_asset_t asset, void *fbx_asset_ptr)
{
	struct fbx_asset *fbx_asset = (struct fbx_asset *)fbx_asset_ptr;

	struct lodge_assets2 *files = lodge_assets2_get_userdata(fbx_assets, USERDATA_FILES);
	ASSERT(files);

	lodge_asset_t file_asset = lodge_assets2_register(files, name);
	if(!file_asset) {
		return false;
	}

	lodge_assets2_add_listener(files, file_asset, fbx_assets, asset);

	const struct lodge_asset_file *file = lodge_assets2_get(files, file_asset);

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

static void lodge_asset_fbx_free_inplace(struct lodge_assets2 *fbx_assets, strview_t name, lodge_asset_t asset, struct fbx_asset *fbx_asset)
{
	struct lodge_assets2 *files = lodge_assets2_get_userdata(fbx_assets, USERDATA_FILES);
	ASSERT(files);

	lodge_assets2_remove_listener_by_name(files, name, fbx_assets, asset);

	fbx_asset_reset(fbx_asset);
}

static struct lodge_ret lodge_plugin_fbx_new_inplace(struct lodge_assets2 *fbx_assets, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	struct lodge_assets2 *files = dependencies[PLUGIN_IDX_FILES];

	lodge_assets2_new_inplace(fbx_assets, &(struct lodge_assets2_desc) {
		.name = strview("fbx"),
		.size = sizeof(struct fbx_asset),
		.new_inplace = &lodge_asset_fbx_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_asset_fbx_free_inplace
	});

	lodge_type_t fbx_asset_type = lodge_type_register_asset(strview("fbx"), fbx_assets);
	ASSERT(fbx_asset_type);

	lodge_assets2_set_userdata(fbx_assets, USERDATA_FILES, files);
	lodge_assets2_set_userdata(fbx_assets, USERDATA_ASSET_TYPE, fbx_asset_type);

	return lodge_success();
}

static void lodge_plugin_fbx_free_inplace(struct lodge_assets2 *fbx_assets)
{
	lodge_assets2_free_inplace(fbx_assets);
}

struct fbx_types lodge_plugin_fbx_get_types(struct lodge_assets2 *fbx_assets)
{
	return (struct fbx_types) {
		.fbx_asset_type = lodge_assets2_get_userdata(fbx_assets, USERDATA_ASSET_TYPE),
	};
}

LODGE_PLUGIN_IMPL(lodge_plugin_fbx)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof(),
		.name = strview("fbx"),
		.new_inplace = &lodge_plugin_fbx_new_inplace,
		.free_inplace = &lodge_plugin_fbx_free_inplace,
		.update = NULL,
		.render = NULL,
		.dependencies = {
			.count = PLUGIN_IDX_MAX,
			.elements = {
				[PLUGIN_IDX_FILES] = {
					.name = strview("files"),
				}
			}
		}
	};
}
