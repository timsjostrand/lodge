#include "lodge_plugin_images.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_res.h"
#include "lodge_image.h"

#define USERDATA_FILES 0

static bool lodge_assets_image_new_inplace(struct lodge_res *res, strview_t name, void *lodge_image_ptr, size_t size)
{
	struct lodge_image *image = (struct lodge_image *)lodge_image_ptr;

	struct lodge_res *files = lodge_res_get_userdata(res, USERDATA_FILES);
	ASSERT(files);

	const struct lodge_res_file *file = lodge_res_get_depend(files, name, (struct lodge_res_handle) {
		.resources = res,
		.id = name,
	});
	if(!file) {
		return false;
	}

	struct lodge_ret ret = lodge_image_new(image, file->data, file->size);
	if(!ret.success) {
		return false;
	}

	return true;
}

static void lodge_assets_image_free_inplace(struct lodge_res *res, strview_t name, struct lodge_image *image)
{
	struct lodge_res *files = lodge_res_get_userdata(res, USERDATA_FILES);
	ASSERT(files);

	lodge_res_release_depend(files, name, (struct lodge_res_handle) {
		.resources = res,
		.id = name,
	});
	lodge_image_free(image);
}

static struct lodge_ret lodge_image_plugin_init(struct lodge_res *images, struct lodge_plugins *plugins)
{
	struct lodge_res *files = lodge_plugins_depend(plugins, images, strview_static("files"));
	if(!files) {
		return lodge_error("Failed to find plugin `files`");
	}

	lodge_res_new_inplace(images, (struct lodge_res_desc) {
		.name = strview_static("images"),
		.size = sizeof(struct lodge_image),
		.new_inplace = &lodge_assets_image_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_assets_image_free_inplace
	} );

	lodge_res_set_userdata(images, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_image_plugin_free(struct lodge_res *images)
{
	lodge_res_free_inplace(images);
}

struct lodge_plugin lodge_images_plugin()
{
	return (struct lodge_plugin) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_res_sizeof(),
		.name = strview_static("images"),
		.init = &lodge_image_plugin_init,
		.free = &lodge_image_plugin_free,
		.update = NULL,
		.render = NULL,
	};
}