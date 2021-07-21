#include "lodge_plugin_shader_sources.h"

#include "strview.h"
#include "txt.h"
#include "strbuf.h"
#include "dynbuf.h"

#include "lodge_assets.h"
#include "lodge_plugin_files.h"
#include "lodge_plugins.h"

#include <ctype.h>

#define USERDATA_FILES 0

struct lodge_shader_source_include
{
	char								name[256];
};

struct lodge_shader_source_includes
{
	size_t								count;
	size_t								capacity;
	struct lodge_shader_source_include	*elements;
};

struct lodge_shader_source
{
	bool								file_loaded;
	struct lodge_shader_source_includes	includes; // FIXME(TS): is this handled automatically by assets dependency system?
	txt_t								resolved;
};

static bool lodge_shader_source_include(struct lodge_shader_source *source, struct lodge_asset_handle source_handle, size_t start, struct lodge_assets *files, strview_t include_file)
{
	const struct lodge_asset_file *file = lodge_assets_get_depend(files, include_file, source_handle);
	ASSERT_OR(file) {
		return false;
	}
	strview_t include_data = strview_make(file->data, file->size);

	if(strview_empty(include_data)) {
		// NOTE(TS): maybe this is a warning and still succeeds? (to keep track of dependencies)
		//shader_error("Include file was empty: " STRVIEW_PRINTF_FMT "\n", STRVIEW_PRINTF_ARG(include_file));
		ASSERT_FAIL("Include file was empty");
		return false;
	}

	source->resolved = txt_insert(source->resolved, start, include_data);
	source->resolved = txt_insert(source->resolved, start + strview_length(include_data), strview_static("\n"));

	// Remember include file
	{
		struct lodge_shader_source_include *include_entry = dynbuf_append_no_init(dynbuf_wrap_stack(source->includes));
		strbuf_set(strbuf_wrap(include_entry->name), include_file);
	}
	
	return true;
}

static int lodge_shader_source_includes_find(struct lodge_shader_source *source, strview_t str)
{
	for(size_t i = 0; i < source->includes.count; i++) {
		if(strbuf_equals(strbuf_wrap(source->includes.elements[i].name), str)) {
			return i;
		}
	}
	return -1;
}

static bool lodge_shader_source_resolve_includes(struct lodge_shader_source *source, strview_t source_name, struct lodge_asset_handle source_handle, struct lodge_assets *files)
{
	dynbuf_clear(dynbuf_wrap_stack(source->includes));

	static const int INCLUDE_LENGTH = 9;
	int retry = 1;

	const strview_t global_include_file = strview_static("global.fxh");
	if(!lodge_shader_source_include(source, source_handle, 0, files, global_include_file)) {
		//shader_debug("Could not include global include file: `" STRVIEW_PRINTF_FMT "`\n", STRVIEW_PRINTF_ARG(global_include_file));
		ASSERT_FAIL("Could not include global include file");
	}

	while(retry) {
		retry = 0;

		size_t count = txt_length(source->resolved); 
		foreach_line(source->resolved, count) {
			if(len == 0) {
				continue;
			}

			size_t post_whitespace = start;
			while(isspace(source->resolved[post_whitespace]) && post_whitespace < count) {
				post_whitespace++;
			}

			const strview_t include_str = strview_static("#include ");
			if(str_begins_with(&source->resolved[post_whitespace], INCLUDE_LENGTH, include_str.s, include_str.length)) {
				txt_t include_file = txt_new(strview_make(&source->resolved[start + INCLUDE_LENGTH], len - INCLUDE_LENGTH));

				// Trim whitespace
				txt_trim(include_file);

				// Strip quotes
				if(txt_begins_with(include_file, strview_static("\""))) {
					txt_delete(include_file, 0, 1);
				}
				if(txt_ends_with(include_file, strview_static("\""))) {
					txt_delete_from_tail(include_file, 1);
				}

				int include_index = lodge_shader_source_includes_find(source, txt_to_strview(include_file));
				if(include_index == -1) {
					//shader_debug("Including file: `%s`\n", include_file);

					// Remove `#include` line
					txt_delete(source->resolved, start, len);

					if(lodge_shader_source_include(source, source_handle, start, files, txt_to_strview(include_file))) {
						retry = 1;
					} else {
						//shader_error("Could not include file: `%s`\n", include_file);
						ASSERT_FAIL("Could not include file");
						txt_free(include_file);
						return false;
					}
				} else {
					//shader_debug("Skipping already included file: `%s`\n", include_file);
					txt_delete(source->resolved, start, len);
				}

				txt_free(include_file);

				if(retry) {
					//shader_debug("Retry include pass\n");
					break;
				}
			}
		}
	};

	{
		char tmp[256];
		strbuf_setf(strbuf_wrap(tmp), "C:/Code/c/lodge-3d/assets-cache/" STRVIEW_PRINTF_FMT, STRVIEW_PRINTF_ARG(source_name));
		FILE *tmp_file = fopen(tmp, "wb");
		if(tmp_file) {
			fwrite(source->resolved, 1, txt_length(source->resolved), tmp_file);
			fclose(tmp_file);
		}
	}

	return true;
}

static bool lodge_shader_source_new_inplace(struct lodge_assets *sources, strview_t name, lodge_asset_id_t id, struct lodge_shader_source *source, size_t data_size)
{
	ASSERT_OR(sizeof(struct lodge_shader_source) == data_size) {
		return false;
	}

	struct lodge_assets *files = lodge_assets_get_userdata(sources, USERDATA_FILES);
	ASSERT_OR(files) {
		return false;
	}

	struct lodge_asset_handle source_handle = { .assets = sources, .id = id };

	const struct lodge_asset_file *file = lodge_assets_get_depend(files, name, source_handle);
	if(!file) {
		return false;
	}

	source->file_loaded = true;
	dynbuf_new_inplace(dynbuf_wrap_stack(source->includes), 8);
	source->resolved = txt_new(strview_make(file->data, file->size));

	bool resolved_includes = lodge_shader_source_resolve_includes(source, name, source_handle, files);
	ASSERT_OR(resolved_includes) {
		return false;
	}

	return true;
}

static void lodge_shader_source_free_inplace(struct lodge_assets *sources, strview_t name, lodge_asset_id_t id, struct lodge_shader_source *source)
{
	struct lodge_assets *files = lodge_assets_get_userdata(sources, USERDATA_FILES);
	ASSERT(files);

	struct lodge_asset_handle source_handle = { .assets = sources, .id = id };

	//
	// Release main source file
	//
	if(source->file_loaded) {
		lodge_assets_release_depend(files, name, source_handle);
	}

	//
	// Release included files
	//
	for(size_t i = 0; i < source->includes.count; i++) {
		lodge_assets_release_depend(files, strbuf_to_strview(strbuf_wrap(source->includes.elements[i].name)), source_handle);
	}

	dynbuf_free_inplace(dynbuf_wrap_stack(source->includes));
	txt_free(source->resolved);
}

static struct lodge_ret lodge_plugin_shader_sources_new_inplace(struct lodge_assets *sources, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	struct lodge_assets *files = lodge_plugins_depend(plugins, sources, strview_static("files"));
	if(!files) {
		return lodge_error("Failed to find plugin `files`");
	}

	lodge_assets_new_inplace(sources, (struct lodge_assets_desc) {
		.name = strview_static("shader_sources"),
		.size = sizeof(struct lodge_shader_source),
		.new_inplace = &lodge_shader_source_new_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_shader_source_free_inplace
	});

	lodge_assets_set_userdata(sources, USERDATA_FILES, files);

	return lodge_success();
}

static void lodge_plugin_shader_sources_free_inplace(struct lodge_assets *sources)
{
	lodge_assets_free_inplace(sources);
}

struct lodge_plugin_desc lodge_plugin_shader_sources()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets_sizeof(),
		.name = strview_static("shader_sources"),
		.new_inplace = &lodge_plugin_shader_sources_new_inplace,
		.free_inplace = &lodge_plugin_shader_sources_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

strview_t lodge_shader_source_get_source(struct lodge_shader_source *source)
{
	return source ? txt_to_strview(source->resolved) : strview_static("");
}
