#include "lodge_plugin_images.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_assets.h"
#include "lodge_image.h"
#include "lodge_json.h"

#include "raw_image.h"

#define USERDATA_FILES 0

static bool lodge_image_desc_from_json(lodge_json_t object, struct lodge_image_desc *desc_out)
{
	double tmp;
	if(lodge_json_object_get_number(object, strview_static("width"), &tmp)) {
		desc_out->width = (uint32_t)tmp;
	} else {
		return false;
	}
	if(lodge_json_object_get_number(object, strview_static("height"), &tmp)) {
		desc_out->height = (uint32_t)tmp;
	} else {
		return false;
	}
	if(lodge_json_object_get_number(object, strview_static("channels"), &tmp)) {
		desc_out->channels = (uint8_t)tmp;
	} else {
		return false;
	}
	if(lodge_json_object_get_number(object, strview_static("bytes_per_channel"), &tmp)) {
		desc_out->bytes_per_channel = (uint8_t)tmp;
	} else {
		return false;
	}
	return true;
}

static bool lodge_assets_image_new_inplace(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, void *lodge_image_ptr, size_t size)
{
	struct lodge_image *image = (struct lodge_image *)lodge_image_ptr;

	struct lodge_assets *files = lodge_assets_get_userdata(assets, USERDATA_FILES);
	ASSERT(files);

	const struct lodge_asset_file *file = lodge_assets_get_depend(files, name, (struct lodge_asset_handle) {
		.assets = assets,
		.id = id,
	});
	if(!file) {
		return false;
	}

	//
	// If .RAW file: look for a raw header:
	//
	// FIXME(TS): Clean this code up
	//
	if(strview_equals(strview_substring_from_end(name, 4), strview_static(".raw"))) {
		char header_file_name[512];
		strbuf_setf(strbuf_wrap(header_file_name), STRVIEW_PRINTF_FMT ".json", STRVIEW_PRINTF_ARG(name));

		const struct lodge_asset_file *header_file = lodge_assets_get_depend(files, strview_wrap(header_file_name), (struct lodge_asset_handle) {
			.assets = assets,
			.id = id,
		});
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

		if(lodge_image_raw_new(image, &raw_desc, file->data, file->size)) {
			free(header_file_data);
			lodge_json_free(header);
			return true;
		}
		free(header_file_data);
		lodge_json_free(header);
	} else {
		struct lodge_ret ret = lodge_image_new(image, file->data, file->size);
		if(ret.success) {
			return true;
		}
	}
	return false;
}

static void lodge_assets_image_free_inplace(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, struct lodge_image *image)
{
	struct lodge_assets *files = lodge_assets_get_userdata(assets, USERDATA_FILES);
	ASSERT(files);

	lodge_assets_release_depend(files, name, (struct lodge_asset_handle) {
		.assets = assets,
		.id = id,
	});
	lodge_image_free(image);
}

static struct lodge_ret lodge_plugin_image_new_inplace(struct lodge_assets *images, struct lodge_plugins *plugins)
{
	struct lodge_assets *files = lodge_plugins_depend(plugins, images, strview_static("files"));
	if(!files) {
		return lodge_error("Failed to find plugin `files`");
	}

	lodge_assets_new_inplace(images, (struct lodge_assets_desc) {
		.name = strview_static("images"),
		.size = sizeof(struct lodge_image),
		.new_inplace = &lodge_assets_image_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_assets_image_free_inplace
	} );

	lodge_assets_set_userdata(images, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_plugin_image_free_inplace(struct lodge_assets *images)
{
	lodge_assets_free_inplace(images);
}

struct lodge_plugin_desc lodge_plugin_images()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets_sizeof(),
		.name = strview_static("images"),
		.new_inplace = &lodge_plugin_image_new_inplace,
		.free_inplace = &lodge_plugin_image_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}