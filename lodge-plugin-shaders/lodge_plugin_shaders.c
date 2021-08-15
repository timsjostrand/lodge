#include "lodge_plugin_shaders.h"

#include "strview.h"
#include "txt.h"
#include "strbuf.h"
#include "array.h"

#include "lodge_res.h"
#include "lodge_shader.h"
#include "lodge_plugin_files.h"
#include "lodge_plugins.h"

#define USERDATA_FILES 0

static bool lodge_shader_source_factory_get(struct lodge_res *shaders, strview_t shader_name, strview_t name, strview_t *source_out)
{
	struct lodge_res *files = lodge_res_get_userdata(shaders, USERDATA_FILES);
	ASSERT(files);
	if(!files) {
		return false;
	}

	lodge_res_id_t shader_res_id = lodge_res_name_to_id(shaders, shader_name);
	ASSERT(lodge_res_id_is_valid(shader_res_id));

	struct lodge_res_handle handle = {
		.resources = shaders,
		.id = shader_res_id,
	};

	const struct lodge_res_file *src = lodge_res_get_depend(files, name, handle);
	ASSERT(src);

	if(src) {
		*source_out = strview_make(src->data, src->size);
		return true;
	}

	return false;
}

static bool lodge_shader_source_factory_release(struct lodge_res *shaders, strview_t shader_name, strview_t name)
{
	struct lodge_res *files = lodge_res_get_userdata(shaders, USERDATA_FILES);
	ASSERT(files);
	if(!files) {
		return false;
	}

	lodge_res_id_t shader_res_id = lodge_res_name_to_id(shaders, shader_name);
	ASSERT(lodge_res_id_is_valid(shader_res_id));

	struct lodge_res_handle handle = {
		.resources = shaders,
		.id = shader_res_id,
	};

	lodge_res_release_depend(files, name, handle);

	return true;
}

static bool lodge_shader_asset_new_inplace(struct lodge_res *shaders, strview_t name, lodge_res_id_t id, lodge_shader_t shader, size_t data_size)
{
	ASSERT(lodge_shader_sizeof() == data_size);

	struct lodge_res *files = lodge_res_get_userdata(shaders, USERDATA_FILES);
	ASSERT(files);

	lodge_shader_new_inplace(shader, name, (struct lodge_shader_source_factory) {
		.userdata = shaders,
		.get_func = &lodge_shader_source_factory_get,
		.release_func = &lodge_shader_source_factory_release
	});

	//
	// FIXME(TS): strbufs => handle will probably crash on reload
	//
	strbuf_t namebuf;
	{
		char tmp[SHADER_FILENAME_MAX];
		namebuf = strbuf_wrap(tmp);
	}

	const struct lodge_res_handle handle = {
		.resources = shaders,
		.id = id,
	};

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".frag", STRVIEW_PRINTF_ARG(name));
		const struct lodge_res_file *src = lodge_res_get_depend(files, strbuf_to_strview(namebuf), handle);
		ASSERT(src);

		if(src && !lodge_shader_set_fragment_source(shader, strview_make(src->data, src->size))) {
			return false;
		}
	}

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".vert", STRVIEW_PRINTF_ARG(name));
		const struct lodge_res_file *src = lodge_res_get_depend(files, strbuf_to_strview(namebuf), handle);
		ASSERT(src);

		if(src && !lodge_shader_set_vertex_source(shader, strview_make(src->data, src->size))) {
			return false;
		}
	}

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".compute", STRVIEW_PRINTF_ARG(name));

		//const struct lodge_asset_file *src = lodge_assets_get_depend(files, strbuf_to_strview(namebuf), handle);
		const struct lodge_asset_file *src = lodge_assets_get(files, strbuf_to_strview(namebuf), handle);
		if(src && !lodge_shader_set_compute_source(shader, strview_make(src->data, src->size))) {
			return false;
		}
	}

	if(!lodge_shader_link(shader)) {
		return false;
	}

	return true;
}

static void lodge_shader_asset_free_inplace(struct lodge_res *shaders, strview_t name, lodge_res_id_t id, lodge_shader_t shader)
{
	struct lodge_res *files = lodge_res_get_userdata(shaders, USERDATA_FILES);
	ASSERT(files);

	lodge_res_clear_dependency(files, (struct lodge_res_handle) {
		.resources = shaders,
		.id = id,
	});

	lodge_shader_free_inplace(shader);
}

static struct lodge_ret lodge_plugin_shaders_new_inplace(struct lodge_res *shaders, struct lodge_plugins *plugins)
{
	struct lodge_res *files = lodge_plugins_depend(plugins, shaders, strview_static("files"));
	if(!files) {
		return lodge_error("Failed to find plugin `files`");
	}

	lodge_res_new_inplace(shaders, (struct lodge_res_desc) {
		.name = strview_static("shaders"),
		.size = lodge_shader_sizeof(),
		.new_inplace = &lodge_shader_asset_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_shader_asset_free_inplace
	});

	lodge_res_set_userdata(shaders, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_plugin_shaders_free_inplace(struct lodge_res *shaders)
{
	lodge_res_free_inplace(shaders);
}

struct lodge_plugin_desc lodge_plugin_shaders()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_res_sizeof(),
		.name = strview_static("shaders"),
		.new_inplace = &lodge_plugin_shaders_new_inplace,
		.free_inplace = &lodge_plugin_shaders_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}
