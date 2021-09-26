#include "lodge_plugin_textures.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_assets2.h"
#include "lodge_texture.h"
#include "lodge_type_asset.h"

enum textures_userdata
{
	USERDATA_IMAGES,
	USERDATA_ASSET_TYPE
};

//
// TODO(TS): implement reload_inplace() to reuse tex id
//

static bool lodge_assets_texture_new_inplace(struct lodge_assets2 *textures, strview_t name, lodge_asset_t asset, void *lodge_texture_ptr)
{
	lodge_texture_t *texture = (lodge_texture_t *)lodge_texture_ptr;

	struct lodge_assets2 *images = lodge_assets2_get_userdata(textures, USERDATA_IMAGES);
	ASSERT_OR(images) { return false; }

	lodge_asset_t image_asset = lodge_assets2_register(images, name);
	ASSERT_OR(image_asset) { return false; }

	lodge_assets2_add_listener(images, image_asset, textures, asset);
	const struct lodge_image *image = lodge_assets2_get(images, image_asset);
	if(!image) {
		return false;
	}

	*texture = lodge_texture_2d_make_from_image(image);

	return true;
}

static void lodge_assets_texture_free_inplace(struct lodge_assets2 *textures, strview_t name, lodge_asset_t asset, lodge_texture_t *texture)
{
	struct lodge_assets2 *images = lodge_assets2_get_userdata(textures, USERDATA_IMAGES);
	ASSERT_OR(images) { return; }

	lodge_assets2_remove_listener_by_name(images, name, textures, asset);

	lodge_texture_reset(*texture);
}

static struct lodge_ret lodge_textures_new_inplace(struct lodge_assets2 *textures, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	struct lodge_assets2 *images = lodge_plugins_depend(plugins, textures, strview("images"));
	if(!images) {
		return lodge_error("Failed to find plugin `images`");
	}

	lodge_assets2_new_inplace(textures, &(struct lodge_assets2_desc) {
		.name = strview("textures"),
		.size = sizeof(lodge_texture_t),
		.new_inplace = &lodge_assets_texture_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_assets_texture_free_inplace
	} );

	lodge_assets2_set_userdata(textures, USERDATA_IMAGES, images);
	lodge_assets2_set_userdata(textures, USERDATA_ASSET_TYPE, lodge_type_register_asset(strview("texture"), textures));

	return lodge_success();
}

static void lodge_textures_free_inplace(struct lodge_assets2 *textures)
{
	lodge_assets2_free_inplace(textures);
}

struct lodge_plugin_desc lodge_plugin_textures()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof(),
		.name = strview("textures"),
		.new_inplace = &lodge_textures_new_inplace,
		.free_inplace = &lodge_textures_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

struct texture_types lodge_plugin_textures_get_types(struct lodge_assets2 *textures)
{
	return (struct texture_types) {
		.texture_asset_type = textures ? lodge_assets2_get_userdata(textures, USERDATA_ASSET_TYPE) : NULL,
	};
}
