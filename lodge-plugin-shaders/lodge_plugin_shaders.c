#include "lodge_plugin_shaders.h"

#include "strview.h"
#include "txt.h"
#include "strbuf.h"
#include "array.h"

#include "lodge_assets2.h"
#include "lodge_shader.h"
#include "lodge_plugin_shader_sources.h"
#include "lodge_plugins.h"
#include "lodge_type_asset.h"

enum lodge_shaders_userdata
{
	USERDATA_SHADER_SOURCES,
	USERDATA_ASSET_TYPE,
};

static bool lodge_shader_asset_new_inplace(struct lodge_assets2 *shaders, strview_t name, lodge_asset_t asset, lodge_shader_t shader)
{
	struct lodge_assets2 *shader_sources = lodge_assets2_get_userdata(shaders, USERDATA_SHADER_SOURCES);
	ASSERT_OR(shader_sources) {
		return false;
	}

	lodge_shader_new_inplace(shader, name);

	//
	// FIXME(TS): strbufs => handle will probably crash on reload
	//
	strbuf_t namebuf;
	{
		char tmp[SHADER_FILENAME_MAX];
		namebuf = strbuf_wrap(tmp);
	}

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".frag", STRVIEW_PRINTF_ARG(name));
		lodge_asset_t source_asset = lodge_assets2_find_by_name(shader_sources, strbuf_to_strview(namebuf));

		if(source_asset) {
			const struct lodge_shader_source *src = lodge_assets2_get(shader_sources, source_asset);
			if(src && !lodge_shader_set_fragment_source(shader, lodge_shader_source_get_source(src))) {
				return false;
			}
			lodge_assets2_add_listener(shader_sources, source_asset, shaders, asset);
		}
	}

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".vert", STRVIEW_PRINTF_ARG(name));
		lodge_asset_t source_asset = lodge_assets2_find_by_name(shader_sources, strbuf_to_strview(namebuf));

		if(source_asset) {
			const struct lodge_shader_source *src = lodge_assets2_get(shader_sources, source_asset);
			if(src && !lodge_shader_set_vertex_source(shader, lodge_shader_source_get_source(src))) {
				return false;
			}
			lodge_assets2_add_listener(shader_sources, source_asset, shaders, asset);
		}
	}

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".compute", STRVIEW_PRINTF_ARG(name));
		lodge_asset_t source_asset = lodge_assets2_find_by_name(shader_sources, strbuf_to_strview(namebuf));

		if(source_asset) {
			const struct lodge_shader_source *src = lodge_assets2_get(shader_sources, source_asset);
			if(src && !lodge_shader_set_compute_source(shader, lodge_shader_source_get_source(src))) {
				return false;
			}
			lodge_assets2_add_listener(shader_sources, source_asset, shaders, asset);
		}
	}

	if(!lodge_shader_link(shader)) {
		return false;
	}

	return true;
}

static void lodge_shader_asset_free_inplace(struct lodge_assets2 *shaders, strview_t name, lodge_asset_t asset, lodge_shader_t shader)
{
	struct lodge_assets2 *shader_sources = lodge_assets2_get_userdata(shaders, USERDATA_SHADER_SOURCES);
	ASSERT(shader_sources);

	char tmp[SHADER_FILENAME_MAX];
	strbuf_t namebuf = strbuf_wrap(tmp);

	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".vert", STRVIEW_PRINTF_ARG(name));
		lodge_assets2_remove_listener_by_name(shader_sources, strbuf_to_strview(namebuf), shaders, asset);
	}
	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".frag", STRVIEW_PRINTF_ARG(name));
		lodge_assets2_remove_listener_by_name(shader_sources, strbuf_to_strview(namebuf), shaders, asset);
	}
	{
		strbuf_setf(namebuf, STRVIEW_PRINTF_FMT ".compute", STRVIEW_PRINTF_ARG(name));
		lodge_assets2_remove_listener_by_name(shader_sources, strbuf_to_strview(namebuf), shaders, asset);
	}

	lodge_shader_free_inplace(shader);
}

static struct lodge_ret lodge_shaders_new_inplace(struct lodge_assets2 *shaders, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	struct lodge_assets2 *shader_sources = lodge_plugins_depend(plugins, shaders, strview_static("shader_sources"));
	if(!shader_sources) {
		return lodge_error("Failed to find plugin `shader_sources`");
	}

	lodge_assets2_new_inplace(shaders, &(struct lodge_assets2_desc) {
		.name = strview_static("shaders"),
		.size = lodge_shader_sizeof(),
		.new_inplace = &lodge_shader_asset_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_shader_asset_free_inplace
	});

	lodge_assets2_set_userdata(shaders, USERDATA_SHADER_SOURCES, shader_sources);
	lodge_assets2_set_userdata(shaders, USERDATA_ASSET_TYPE, lodge_type_register_asset(strview("shader"), shaders));

	return lodge_success();
}

static void lodge_shaders_free_inplace(struct lodge_assets2 *shaders)
{
	lodge_assets2_free_inplace(shaders);
}

struct shader_types lodge_plugin_shaders_get_types(struct lodge_assets2 *shaders)
{
	return (struct shader_types) {
		.shader_asset_type = shaders ? lodge_assets2_get_userdata(shaders, USERDATA_ASSET_TYPE) : NULL,
	};
}

LODGE_PLUGIN_IMPL(lodge_plugin_shaders)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof(),
		.name = strview_static("shaders"),
		.new_inplace = &lodge_shaders_new_inplace,
		.free_inplace = &lodge_shaders_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}
