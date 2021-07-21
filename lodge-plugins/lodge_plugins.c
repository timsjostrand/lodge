#include "lodge_plugins.h"

#include "lodge.h"
#include "log.h"
#include "membuf.h"
#include "core_argv.h"

#include <string.h>

// FIXME(TS): Just for `_plugin()` funcs
#include "lodge_window.h"
#include "lodge_gfx.h"
#include "game.h"
#include "lodge_vfs.h"
#include "lodge_plugin_vfs.h"
#include "lodge_plugin_types.h"
#include "lodge_plugin_env.h"
#include "lodge_plugin_files.h"
#include "lodge_plugin_fbx.h"
#include "lodge_plugin_images.h"
#include "lodge_plugin_textures.h"
#include "lodge_plugin_shader_sources.h"
#include "lodge_plugin_shaders.h"
#include "lodge_plugin_scene_renderer.h"
#include "lodge_plugin_debug_draw.h"
#include "lodge_plugin_terrain.h"
#include "lodge_plugin_water.h"
#include "lodge_plugin_editor.h"
#include "lodge_plugin_renderdoc.h"

#define LODGE_PLUGINS_MAX				128
#define LODGE_PLUGIN_DEPENDENCIES_MAX	32

struct lodge_plugin_meta
{
	struct lodge_plugin_desc			*deps[LODGE_PLUGIN_DEPENDENCIES_MAX];
	size_t								deps_count;
};

struct lodge_plugins
{
	bool								running;
	struct lodge_plugin_desc			list[LODGE_PLUGINS_MAX];
	struct lodge_plugin_meta			meta[LODGE_PLUGINS_MAX];
	size_t								offsets[LODGE_PLUGINS_MAX];
	uint32_t							count;
	char								*data;
	size_t								data_size;
	float								delta_time_factor;
	struct lodge_plugins_frame_times	frame_times;

	const struct lodge_argv				*args;
};

static struct lodge_plugin_desc* lodge_plugins_find_plugin_by_data(struct lodge_plugins *plugins, lodge_plugin_data_t data)
{
	char *cur = plugins->data;
	for(uint32_t i = 0; i < plugins->count; i++) {
		struct lodge_plugin_desc *plugin = &plugins->list[i];
		if(data == cur) {
			return plugin;
		}
		cur += plugin->size;
	}
	return NULL;
}

static void lodge_plugins_add_dependency(struct lodge_plugins *plugins, size_t plugin_index, struct lodge_plugin_desc *dependee)
{
	struct lodge_plugin_meta *meta = &plugins->meta[plugin_index];

	for(size_t i = 0, count = meta->deps_count; i < count; i++) {
		if(meta->deps[i] == dependee) {
			return;
		}
	}

	meta->deps[meta->deps_count++] = dependee;
}

static bool lodge_plugins_remove_dependency(struct lodge_plugins *plugins, size_t plugin_index, struct lodge_plugin_desc *dependee)
{
	struct lodge_plugin_meta *meta = &plugins->meta[plugin_index];

	for(size_t i = 0, count = meta->deps_count; i < count; i++) {
		if(meta->deps[i] == dependee) {
			membuf_delete_swap_tail(membuf_wrap(meta->deps), &meta->deps_count, i);
			return true;
		}
	}

	return false;
}

static bool lodge_plugins_is_dependency_impl(struct lodge_plugins *plugins, size_t plugin_index, struct lodge_plugin_desc *dependee)
{
	struct lodge_plugin_meta *meta = &plugins->meta[plugin_index];

	for(size_t i = 0, count = meta->deps_count; i < count; i++) {
		if(meta->deps[i] == dependee) {
			return true;
		}
	}

	return false;
}

static void lodge_plugins_register_frame(struct lodge_plugins_frame_times *f, float delta_time)
{
	f->frames++;
	f->frame_time_min = min(delta_time, f->frame_time_min);
	f->frame_time_max = max(delta_time, f->frame_time_max);
	f->frame_time_sum += delta_time;
	
	if (lodge_window_get_time() - f->last_frame_report >= 1000.0) {
		f->last_frame_report = lodge_window_get_time();
		f->frame_time_avg = f->frame_time_sum / (float) f->frames;

#if 1
		// TODO(TS): reimplement frame time callback
		//if(f->callback != NULL) {
		//    f->callback(f);
		//}
#else
		debugf("Plugins", "FPS: %.0f\n", f->frame_time_avg);
#endif

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
	for(uint32_t i = plugins->count - 1; i >= 0; i--) {
		struct lodge_plugin_desc *plugin = &plugins->list[i];

		const size_t offset = plugins->offsets[i];
		char *plugin_data = &plugins->data[offset];

		debugf("Plugins", "Freeing plugin `" STRVIEW_PRINTF_FMT "`\n", STRVIEW_PRINTF_ARG(plugin->name));

		if(plugin->free_inplace) {
			plugin->free_inplace(plugin_data);
		}
	}

	free(plugins->data);
	free(plugins);
}

uint32_t lodge_plugins_get_count(const struct lodge_plugins *plugins)
{
	return plugins->count;
}

struct lodge_plugin_find_ret
{
	bool		success;
	void		*data;
	uint32_t	index;
};

struct lodge_plugin_find_ret lodge_plugins_find_by_name(struct lodge_plugins *plugins, strview_t name)
{
	char *cur = plugins->data;
	for(uint32_t i = 0; i < plugins->count; i++) {
		struct lodge_plugin_desc *plugin = &plugins->list[i];
		if(strview_equals(plugin->name, name)) {
			return (struct lodge_plugin_find_ret) {
				.success = true,
				.data = cur,
				.index = i,
			};
		}
		cur += plugin->size;
	}

	return (struct lodge_plugin_find_ret) {
		.success = false,
		.data = NULL,
		.index = 0,
	};
}

lodge_plugin_data_t lodge_plugins_depend(struct lodge_plugins *plugins, lodge_plugin_data_t dependee, strview_t name)
{
	ASSERT(plugins->data);

	struct lodge_plugin_desc *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	if(!dependee_plugin) {
		ASSERT_FAIL("Failed to find dependee");
		return NULL;
	}

	struct lodge_plugin_find_ret find_ret = lodge_plugins_find_by_name(plugins, name);
	if(find_ret.success) {
		lodge_plugins_add_dependency(plugins, find_ret.index, dependee_plugin);
		return find_ret.data;
	}

	// TODO(TS): remove `dependee` from all plugins when unloaded

	return NULL;
}

bool lodge_plugins_undepend(struct lodge_plugins *plugins, lodge_plugin_data_t dependee, strview_t name)
{
	ASSERT_OR(plugins && plugins->data) {
		return false;
	}

	struct lodge_plugin_desc *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	ASSERT_OR(dependee_plugin) {
		return false;
	}

	struct lodge_plugin_find_ret find_ret = lodge_plugins_find_by_name(plugins, name);
	if(find_ret.success) {
		if(lodge_plugins_remove_dependency(plugins, find_ret.index, dependee_plugin)) {
			return true;
		}
	}

	return false;
}

bool lodge_plugins_is_dependency(struct lodge_plugins *plugins, lodge_plugin_data_t dependee, strview_t name)
{
	struct lodge_plugin_desc *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	if(!dependee_plugin) {
		return false;
	}

	struct lodge_plugin_find_ret find_ret = lodge_plugins_find_by_name(plugins, name);
	if(find_ret.success) {
		if(lodge_plugins_is_dependency_impl(plugins, find_ret.index, dependee_plugin)) {
			return true;
		}
	}

	return false;
}

void lodge_plugins_append(struct lodge_plugins *plugins, struct lodge_plugin_desc plugin)
{
	plugins->list[plugins->count++] = plugin;
}

struct lodge_ret lodge_plugins_find(struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	// TODO(TS): find plugins either by looking for dynamic libraries in filesystem or static list

	plugins->args = args;

	lodge_plugins_append(plugins, lodge_plugin_types());
	lodge_plugins_append(plugins, lodge_plugin_vfs());
	lodge_plugins_append(plugins, lodge_plugin_windows());
	lodge_plugins_append(plugins, lodge_plugin_env());
	lodge_plugins_append(plugins, lodge_plugin_files());
	lodge_plugins_append(plugins, lodge_plugin_fbx());
	lodge_plugins_append(plugins, lodge_plugin_images());
	lodge_plugins_append(plugins, lodge_plugin_textures());
	lodge_plugins_append(plugins, lodge_plugin_shader_sources());
	lodge_plugins_append(plugins, lodge_plugin_shaders());
	lodge_plugins_append(plugins, lodge_scene_renderer_plugin());
	lodge_plugins_append(plugins, lodge_plugin_debug_draw());
	lodge_plugins_append(plugins, lodge_plugin_terrain());
	lodge_plugins_append(plugins, lodge_plugin_water());
	lodge_plugins_append(plugins, lodge_plugin_editor());
	lodge_plugins_append(plugins, lodge_plugin_renderdoc());
	lodge_plugins_append(plugins, game_plugin());

	for(uint32_t i = 0; i < plugins->count; i++) {
		debugf("Plugins", "Found plugin: `" STRVIEW_PRINTF_FMT "` (%.1f kB)\n",
			STRVIEW_PRINTF_ARG(plugins->list[i].name),
			plugins->list[i].size / 1024.0f
		);
	}

	return lodge_success();
}

struct lodge_ret lodge_plugins_init(struct lodge_plugins *plugins)
{
	const uint32_t count = plugins->count;
	
	/* Preallocate plugins data */
	plugins->data_size = 0;
	for(uint32_t i = 0; i < count; i++) {
		struct lodge_plugin_desc *plugin = &plugins->list[i];
		plugins->data_size += plugin->size;
	}

	if(plugins->data_size > 0) {
		plugins->data = malloc(plugins->data_size);
		if(plugins->data) {
			memset(plugins->data, 0, plugins->data_size);
		} else {
			return lodge_error("Out of memory");
		}
	}

	/* Init plugins */
	char *cur = plugins->data;

	for(int i = 0; i < count; i++) {
		struct lodge_plugin_desc *plugin = &plugins->list[i];

		if(plugin->version != LODGE_PLUGIN_VERSION) {
			errorf("Plugins", "Incorrect plugin version: %d (expected: %d)\n", plugin->version, LODGE_PLUGIN_VERSION);
			return lodge_error("Incorrect plugin version");
		}

		memset(cur, 0, plugin->size);

		if(plugin->new_inplace) {
			debugf("Plugins", "Initializing `%s`...\n", plugin->name.s);
			struct lodge_ret init_ret = plugin->new_inplace(cur, plugins, plugins->args);
			if(!init_ret.success) {
				errorf("Plugins", "Error when initializing plugin `%s`: %s\n", plugin->name.s, init_ret.message.s);
				return init_ret;
			}
		}

		/* Update offsets */ 
		plugins->offsets[i] = (i == 0 ? 0 : plugins->offsets[i-1] + plugins->list[i-1].size);

		cur += plugin->size;
	}

	/* TODO(TS): register reload callbacks (call init and free in correct order) */
	return lodge_success();
}

void lodge_plugins_run(struct lodge_plugins *plugins)
{
	plugins->running = true;

	/* Main loop */
	while(plugins->running)
	{
		const int count = plugins->count;
		const double before = lodge_window_get_time(); // FIXME(TS): shoud use lodge_get_time

		struct lodge_plugins_frame_times *frame_times = &plugins->frame_times;

		/* Delta-time. */
		float delta_time = 0;
		if(frame_times->last_frame != 0) {
			delta_time = (float)((before - frame_times->last_frame) * plugins->delta_time_factor);
		}
		frame_times->last_frame = before;

		/* Update plugins */
		{
			char *cur = plugins->data;
			for(int i = 0; i < count; i++) {
				struct lodge_plugin_desc *plugin = &plugins->list[i];
				if(plugin->update) {
					plugin->update(cur, delta_time);
				}
				cur += plugin->size;
			}
		}

		/* Render plugins */
		{
			char *cur = plugins->data;
			for(int i = 0; i < count; i++) {
				struct lodge_plugin_desc *plugin = &plugins->list[i];
				if(plugin->render) {
					plugin->render(cur);
				}
				cur += plugin->size;
			}
		}

		/* Register that a frame has been completed. */
		lodge_plugins_register_frame(frame_times, (float)(lodge_window_get_time() - before)); // FIXME(TS): should use lodge_get_time
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
	return plugins->frame_times;
}

const struct lodge_plugin_desc* lodge_plugins_get_desc(const struct lodge_plugins *plugins, size_t index)
{
	return (index >= 0 && index < plugins->count) ? &plugins->list[index] : NULL;
}

uint32_t lodge_plugins_get_dependencies_count(const struct lodge_plugins *plugins, size_t index)
{
	return (index >= 0 && index < plugins->count) ? plugins->meta[index].deps_count : 0;
}