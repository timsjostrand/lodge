#include "lodge_plugins.h"

#include "lodge.h"
#include "log.h"
#include "membuf.h"

#include "lodge_time.h"
#include "lodge_argv.h"
#include "lodge_plugin.h"
#include "lodge_vfs.h"

#include <string.h>

#define LODGE_PLUGINS_MAX				128
#define LODGE_PLUGIN_DEPENDENCIES_MAX	32

struct lodge_loaded_plugin
{
	struct lodge_plugin_desc			*desc;
	bool								initialized;

	char								*data;

	struct lodge_loaded_plugin			*deps[LODGE_PLUGIN_DEPENDENCIES_MAX];
	size_t								deps_count;
};

struct lodge_plugins
{
	bool								running;

	struct lodge_plugin_desc			descriptions[LODGE_PLUGINS_MAX];
	uint32_t							descriptions_count;
	
	struct lodge_loaded_plugin			loaded[LODGE_PLUGINS_MAX];
	size_t								loaded_count;
	
	float								delta_time_factor;
	struct lodge_plugins_frame_times	frame_times;
	struct lodge_plugins_frame_times	last_frame_times;

	const struct lodge_argv				*args;
};

static struct lodge_plugin_desc* lodge_plugins_find_desc_by_name(struct lodge_plugins *plugins, strview_t name)
{
	for(uint32_t i = 0; i < plugins->descriptions_count; i++) {
		struct lodge_plugin_desc *desc = &plugins->descriptions[i];
		if(strview_equals(desc->name, name)) {
			return desc;
		}
	}
	return NULL;
}

static struct lodge_loaded_plugin* lodge_plugins_find_by_name(struct lodge_plugins *plugins, strview_t name)
{
	for(uint32_t i = 0; i < plugins->loaded_count; i++) {
		struct lodge_loaded_plugin *plugin = &plugins->loaded[i];
		if(strview_equals(plugin->desc->name, name)) {
			return plugin;
		}
	}
	return NULL;
}

static struct lodge_ret lodge_plugin_try_initialize(struct lodge_plugins *plugins, struct lodge_plugin_desc *desc)
{
	struct lodge_loaded_plugin *plugin = lodge_plugins_find_by_name(plugins, desc->name);
	if(!plugin) {
		plugin = membuf_append_no_init(membuf(plugins->loaded), &plugins->loaded_count);

		plugin->data = NULL;
		plugin->deps_count = 0;
		plugin->desc = desc;
		plugin->initialized = false;
	}

	if(!plugin->initialized) {
		plugin->data = calloc(1, plugin->desc->size);

		//
		// HACK(TS): all plugins depend on the "types" plugin to do static init correctly
		//
		void *types = lodge_plugins_depend(plugins, plugin->data, strview("types"));
		ASSERT(types);

		//
		// Check if we need to mount the plugin directory from source tree.
		//
		{
			struct lodge_static_mounts *static_mounts = &plugin->desc->static_mounts;
			for(size_t i=0, count=static_mounts->count; i<count; i++) {
				struct lodge_static_mount *mount = &static_mounts->elements[i];
		
				struct lodge_vfs *vfs = lodge_plugins_depend(plugins, plugin->data, strview("vfs"));
				ASSERT(vfs);
				if(vfs) {
					lodge_vfs_mount(vfs, mount->dst_point, mount->src_dir);
				}
			}
		}

		if(plugin->desc->new_inplace) {
			debugf("Plugins", "Initializing `" STRVIEW_PRINTF_FMT "`...\n", STRVIEW_PRINTF_ARG(plugin->desc->name));
			struct lodge_ret init_ret = plugin->desc->new_inplace(plugin->data, plugins, plugins->args);
			if(!init_ret.success) {
				errorf("Plugins", "Error when initializing plugin `" STRVIEW_PRINTF_FMT "`: " STRVIEW_PRINTF_FMT "\n",
					STRVIEW_PRINTF_ARG(plugin->desc->name),
					STRVIEW_PRINTF_ARG(init_ret.message)
				);
				return init_ret;
			}
		}

		plugin->initialized = true;
	}

	return lodge_success();
}

static void lodge_plugin_try_free(struct lodge_plugins *plugins, struct lodge_loaded_plugin *plugin)
{
	ASSERT_OR(plugins && plugin) { return; }

	if(plugin->initialized) {

		// FIXME(TS): kind of brute force
		for(int i=0; i<plugin->deps_count; i++) {
			lodge_plugin_try_free(plugins, plugin->deps[i]);
		}

		debugf("Plugins", "Uninitializing `" STRVIEW_PRINTF_FMT "`...\n", STRVIEW_PRINTF_ARG(plugin->desc->name));
		if(plugin->desc->free_inplace) {
			plugin->desc->free_inplace(plugin->data);
		}
		free(plugin->data);
		plugin->data = NULL;
		plugin->initialized = false;
	}
}

static struct lodge_loaded_plugin* lodge_plugins_find_plugin_by_data(struct lodge_plugins *plugins, void *data)
{
	for(uint32_t i = 0; i < plugins->loaded_count; i++) {
		struct lodge_loaded_plugin *plugin = &plugins->loaded[i];
		if(plugin->data == data) {
			return plugin;
		}
	}
	return NULL;
}

static void lodge_plugins_add_dependency(struct lodge_plugins *plugins, struct lodge_loaded_plugin *plugin, struct lodge_loaded_plugin *dependee)
{
	for(size_t i = 0, count = plugin->deps_count; i < count; i++) {
		if(plugin->deps[i] == dependee) {
			return;
		}
	}

	plugin->deps[plugin->deps_count++] = dependee;

	if(plugin->deps_count == 1) {
		struct lodge_ret init_ret = lodge_plugin_try_initialize(plugins, plugin->desc);
		if(!init_ret.success) {
			ASSERT_FAIL("API does not handle this");
		}
	}
}

static bool lodge_plugins_remove_dependency(struct lodge_plugins *plugins, struct lodge_loaded_plugin *plugin, struct lodge_loaded_plugin *dependee)
{
	for(size_t i = 0, count = plugin->deps_count; i < count; i++) {
		if(plugin->deps[i] == dependee) {
			membuf_delete_swap_tail(membuf_wrap(plugin->deps), &plugin->deps_count, i);

			if(plugin->deps_count == 0) {
				lodge_plugin_try_free(plugins, plugin);
			}

			return true;
		}
	}

	return false;
}

static bool lodge_plugins_is_dependency_impl(struct lodge_plugins *plugins, struct lodge_loaded_plugin *plugin, struct lodge_loaded_plugin *dependee)
{
	for(size_t i = 0, count = plugin->deps_count; i < count; i++) {
		if(plugin->deps[i] == dependee) {
			return true;
		}
	}

	return false;
}

static void lodge_plugins_register_frame(struct lodge_plugins *plugins, float delta_time)
{
	struct lodge_plugins_frame_times *f = &plugins->frame_times;

	f->frames++;
	f->frame_time_min = min(delta_time, f->frame_time_min);
	f->frame_time_max = max(delta_time, f->frame_time_max);
	f->frame_time_sum += delta_time;
	
	if(lodge_timestamp_elapsed_ms(f->last_frame_report) >= 1000.0) {
		f->last_frame_report = lodge_timestamp_get();
		f->frame_time_avg = f->frame_time_sum / (float) f->frames;

		plugins->last_frame_times = *f;

		f->frame_time_max = -FLT_MAX;
		f->frame_time_min = FLT_MAX;
		f->frame_time_sum = 0;
		f->frames = 0;
	}
}

struct lodge_plugins* lodge_plugins_new()
{
	struct lodge_plugins *plugins = (struct lodge_plugins *) calloc(1, sizeof(struct lodge_plugins));
	if(!plugins) {
		return NULL;
	}
	plugins->delta_time_factor = 1.0;
	return plugins;
}

void lodge_plugins_free(struct lodge_plugins *plugins)
{
	for(int64_t i = plugins->loaded_count - 1; i >= 0; i--) {
		struct lodge_loaded_plugin *plugin = &plugins->loaded[i];
		lodge_plugin_try_free(plugins, plugin);
	}
	free(plugins);
}

uint32_t lodge_plugins_get_count(const struct lodge_plugins *plugins)
{
	return plugins->loaded_count;
}

void* lodge_plugins_depend(struct lodge_plugins *plugins, void *dependee, strview_t name)
{
	ASSERT_OR(plugins && dependee) { return NULL; }

	struct lodge_loaded_plugin *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	if(!dependee_plugin) {
		ASSERT_FAIL("Failed to find dependee");
		return NULL;
	}

	struct lodge_loaded_plugin *plugin = lodge_plugins_find_by_name(plugins, name);
	if(!plugin) {
		struct lodge_plugin_desc *plugin_desc = lodge_plugins_find_desc_by_name(plugins, name);
		ASSERT_OR(plugin_desc) { return NULL; }
		struct lodge_ret plugin_init_ret = lodge_plugin_try_initialize(plugins, plugin_desc);
		ASSERT_OR(plugin_init_ret.success) { return NULL; }
		plugin = lodge_plugins_find_by_name(plugins, name);
	}
	if(plugin) {
		if(plugin != dependee_plugin) {
			lodge_plugins_add_dependency(plugins, plugin, dependee_plugin);
		}
		return plugin->data;
	}

	// TODO(TS): remove `dependee` from all plugins when unloaded

	return NULL;
}

bool lodge_plugins_undepend(struct lodge_plugins *plugins, void *dependee, strview_t name)
{
	ASSERT_OR(plugins && dependee) { return false; }

	struct lodge_loaded_plugin *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	ASSERT_OR(dependee_plugin) {
		return false;
	}

	struct lodge_loaded_plugin *plugin = lodge_plugins_find_by_name(plugins, name);
	if(plugin) {
		if(lodge_plugins_remove_dependency(plugins, plugin, dependee_plugin)) {
			return true;
		}
	}

	return false;
}

bool lodge_plugins_is_dependency(struct lodge_plugins *plugins, void *dependee, strview_t name)
{
	struct lodge_loaded_plugin *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	if(!dependee_plugin) {
		return false;
	}

	struct lodge_loaded_plugin *plugin = lodge_plugins_find_by_name(plugins, name);
	if(plugin) {
		if(lodge_plugins_is_dependency_impl(plugins, plugin, dependee_plugin)) {
			return true;
		}
	}

	return false;
}

struct lodge_ret lodge_plugins_find(struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	//
	// TODO(TS): also find plugins by looking for shared libs
	//

	plugins->args = args;

	//
	// Find statically compiled plugins.
	//
	for(uint32_t i = 0; i < lodge_plugin_registry_count; i++) {
		plugins->descriptions[plugins->descriptions_count++] = lodge_plugin_registry[i]();

		debugf("Plugins", "Found plugin: `" STRVIEW_PRINTF_FMT "` (%.1f kB)\n",
			STRVIEW_PRINTF_ARG(plugins->descriptions[i].name),
			plugins->descriptions[i].size / 1024.0f
		);
	}

	return lodge_success();
}

struct lodge_ret lodge_plugins_init(struct lodge_plugins *plugins)
{
	if(plugins->args->positionals.count > 0) {
		for(const struct lodge_argv_positional* it = lodge_argv_positional_it_begin(plugins->args); it; it = lodge_argv_positional_it_next(plugins->args, it)) {
			struct lodge_plugin_desc *default_plugin = lodge_plugins_find_desc_by_name(plugins, it->key);
			if(!default_plugin) {
				return lodge_error("Failed to find default plugin");
			}
			struct lodge_ret init_default_ret = lodge_plugin_try_initialize(plugins, default_plugin);
			if(!init_default_ret.success) {
				return init_default_ret;
			}
		}
	} else {
		struct lodge_plugin_desc *default_plugin = lodge_plugins_find_desc_by_name(plugins, lodge_argv_get_str(plugins->args, strview("plugin"), strview("editor")));
		if(!default_plugin) {
			return lodge_error("Failed to find default plugin");
		}
		struct lodge_ret init_default_ret = lodge_plugin_try_initialize(plugins, default_plugin);
		if(!init_default_ret.success) {
			return init_default_ret;
		}
	}

	return lodge_success();
}

void lodge_plugins_run(struct lodge_plugins *plugins)
{
	plugins->running = true;

	/* Main loop */
	while(plugins->running) {
		const int count = plugins->loaded_count;
		lodge_timestamp_t before = lodge_timestamp_get();

		struct lodge_plugins_frame_times *frame_times = &plugins->frame_times;

		/* Delta-time. */
		float delta_time = 0;
		if(frame_times->last_frame != 0) {
			delta_time = lodge_timestamp_elapsed_ms(frame_times->last_frame) * plugins->delta_time_factor;
		}
		frame_times->last_frame = before;

		/* Update plugins */
		{
			for(int i = 0; i < count; i++) {
				struct lodge_loaded_plugin *plugin = &plugins->loaded[i];
				if(!plugin->initialized) {
					continue;
				}
				if(plugin->desc->update) {
					plugin->desc->update(plugin->data, delta_time);
				}
			}
		}

		/* Render plugins */
		{
			for(int i = 0; i < count; i++) {
				struct lodge_loaded_plugin *plugin = &plugins->loaded[i];
				if(!plugin->initialized) {
					continue;
				}
				if(plugin->desc->render) {
					plugin->desc->render(plugin->data);
				}
			}
		}

		/* Register that a frame has been completed. */
		lodge_plugins_register_frame(plugins, lodge_timestamp_elapsed_ms(before));
	}
}

void lodge_plugins_set_running(struct lodge_plugins *plugins, bool running)
{
	plugins->running = running;
}

void lodge_plugins_set_delta_time_factor(struct lodge_plugins *plugins, float delta_time_factor)
{
	plugins->delta_time_factor = delta_time_factor;
}

struct lodge_plugins_frame_times lodge_plugins_get_frame_times(struct lodge_plugins *plugins)
{
	return plugins->last_frame_times;
}

const struct lodge_plugin_desc* lodge_plugins_get_desc(const struct lodge_plugins *plugins, size_t index)
{
	return (index >= 0 && index < plugins->descriptions_count) ? plugins->loaded[index].desc : NULL;
}

uint32_t lodge_plugins_get_dependencies_count(const struct lodge_plugins *plugins, size_t index)
{
	return (index >= 0 && index < plugins->descriptions_count) ? plugins->loaded[index].deps_count : 0;
}