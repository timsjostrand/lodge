#include "lodge_plugin_textures.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_assets.h"
#include "lodge_texture.h"

#define USERDATA_IMAGES 0

// TODO(TS): implement reload_inplace() to reuse tex id

static bool lodge_assets_texture_new_inplace(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, void *lodge_texture_ptr, size_t size)
{
	lodge_texture_t *texture = (lodge_texture_t *)lodge_texture_ptr;

	struct lodge_assets *images = lodge_assets_get_userdata(assets, USERDATA_IMAGES);
	ASSERT(images);

	const struct lodge_image *image = lodge_assets_get_depend(images, name, (struct lodge_asset_handle) {
		.assets = assets,
		.id = id,
	});
	if(!image) {
		return false;
	}

	*texture = lodge_texture_2d_make_from_image(image);

	return true;
}

static void lodge_assets_texture_free_inplace(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, lodge_texture_t *texture)
{
	struct lodge_assets *images = lodge_assets_get_userdata(assets, USERDATA_IMAGES);
	ASSERT(images);

	lodge_assets_release_depend(images, name, (struct lodge_asset_handle) {
		.assets = assets,
		.id = id,
	});
	lodge_texture_reset(*texture);
}

static struct lodge_ret lodge_plugin_textures_new_inplace(struct lodge_assets *textures, struct lodge_plugins *plugins)
{
	struct lodge_assets *images = lodge_plugins_depend(plugins, textures, strview_static("images"));
	if(!images) {
		return lodge_error("Failed to find plugin `images`");
	}

	lodge_assets_new_inplace(textures, (struct lodge_assets_desc) {
		.name = strview_static("textures"),
		.size = sizeof(lodge_texture_t),
		.new_inplace = &lodge_assets_texture_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_assets_texture_free_inplace
	} );

	lodge_assets_set_userdata(textures, USERDATA_IMAGES, images);

	return lodge_success();
}

static void lodge_plugin_texture_free_inplace(struct lodge_assets *textures)
{
	lodge_assets_free_inplace(textures);
}

struct lodge_plugin_desc lodge_plugin_textures()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets_sizeof(),
		.name = strview_static("textures"),
		.new_inplace = &lodge_plugin_textures_new_inplace,
		.free_inplace = &lodge_plugin_texture_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}