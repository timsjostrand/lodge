#include "lodge_plugin_images.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_assets2.h"
#include "lodge_image.h"
#include "lodge_json.h"

#include "lodge_image_raw.h"

#define USERDATA_FILES 0

enum lodge_plugin_idx
{
	PLUGIN_IDX_FILES = 0,
	PLUGIN_IDX_MAX
};

//
// TODO(TS): use property object api instead
//
static bool lodge_image_desc_from_json(lodge_json_t object, struct lodge_image_desc *desc_out)
{
	double tmp;
	if(lodge_json_object_get_number(object, strview("width"), &tmp)) {
		desc_out->width = (uint32_t)tmp;
	} else {
		return false;
	}
	if(lodge_json_object_get_number(object, strview("height"), &tmp)) {
		desc_out->height = (uint32_t)tmp;
	} else {
		return false;
	}
	if(lodge_json_object_get_number(object, strview("channels"), &tmp)) {
		desc_out->channels = (uint8_t)tmp;
	} else {
		return false;
	}
	if(lodge_json_object_get_number(object, strview("bytes_per_channel"), &tmp)) {
		desc_out->bytes_per_channel = (uint8_t)tmp;
	} else {
		return false;
	}
	return true;
}

static bool lodge_image_asset_new_inplace(struct lodge_assets2 *images, strview_t name, lodge_asset_t image_asset, void *lodge_image_ptr)
{
	struct lodge_image *image = (struct lodge_image *)lodge_image_ptr;

	struct lodge_assets2 *files = lodge_assets2_get_userdata(images, USERDATA_FILES);
	ASSERT(files);

	lodge_asset_t file_asset = lodge_assets2_register(files, name);
	if(!file_asset) {
		return false;
	}

	lodge_assets2_add_listener(files, file_asset, images, image_asset);
	const struct lodge_asset_file *file = lodge_assets2_get(files, file_asset);
	if(!file) {
		return false;
	}

	//
	// If .RAW file: look for a raw header:
	//
	// FIXME(TS): Clean this code up
	//
	if(strview_ends_with(name, strview(".raw"))) {
		char header_file_name[512];
		strbuf_setf(strbuf_wrap(header_file_name), STRVIEW_PRINTF_FMT ".json", STRVIEW_PRINTF_ARG(name));

		lodge_asset_t header_file_asset = lodge_assets2_register(files, strview_wrap(header_file_name));
		if(!header_file_asset) {
			return false;
		}

		lodge_assets2_add_listener(files, header_file_asset, images, image_asset);
		const struct lodge_asset_file *header_file = lodge_assets2_get(files, header_file_asset);
		if(!header_file) {
			return false;
		}

		// Need to end with \0 because of the JSON lib
		size_t header_file_size = header_file->size + 1; 
		char* header_file_data = malloc(header_file_size);
		memcpy(header_file_data, header_file->data, header_file->size);
		header_file_data[header_file_size - 1] = '\0';

		lodge_json_t header = lodge_json_from_string(strview_make(header_file_data, header_file->size));
		ASSERT(header);
		if(!header) {
			free(header_file_data);
			return false;
		}

		struct lodge_image_desc raw_desc = { 0 };
		if(!lodge_image_desc_from_json(header, &raw_desc)) {
			free(header_file_data);
			lodge_json_free(header);
			return false;
		}

		if(lodge_image_raw_new(image, &raw_desc, (const uint8_t*)file->data, file->size)) {
			free(header_file_data);
			lodge_json_free(header);
			return true;
		}
		free(header_file_data);
		lodge_json_free(header);
	} else {
		struct lodge_ret ret = lodge_image_new(image, (const uint8_t*)file->data, file->size);
		if(ret.success) {
			return true;
		}
	}
	return false;
}

static void lodge_image_asset_free_inplace(struct lodge_assets2 *images, strview_t name, lodge_asset_t image_asset, struct lodge_image *image)
{
	struct lodge_assets2 *files = lodge_assets2_get_userdata(images, USERDATA_FILES);
	ASSERT(files);

	lodge_assets2_remove_listener_by_name(files, name, images, image_asset);
	
	//lodge_assets2_remove_listener_by_name(files, header_file_name);

	lodge_image_free(image);
}

static struct lodge_ret lodge_images_new_inplace(struct lodge_assets2 *images, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	struct lodge_assets *files = dependencies[PLUGIN_IDX_FILES];

	lodge_assets2_new_inplace(images, &(struct lodge_assets2_desc) {
		.name = strview("images"),
		.size = sizeof(struct lodge_image),
		.new_inplace = &lodge_image_asset_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_image_asset_free_inplace
	} );

	lodge_assets2_set_userdata(images, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_images_free_inplace(struct lodge_assets2 *images)
{
	lodge_assets2_free_inplace(images);
}

LODGE_PLUGIN_IMPL(lodge_plugin_images)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof(),
		.name = strview("images"),
		.new_inplace = &lodge_images_new_inplace,
		.free_inplace = &lodge_images_free_inplace,
		.update = NULL,
		.render = NULL,
		.dependencies ={
			.count = PLUGIN_IDX_MAX,
			.elements = {
				[PLUGIN_IDX_FILES] = {
					.name = strview("files")
				}
			}
		}
	};
}