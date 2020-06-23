#include "lodge_plugins.h"

#include "lodge.h"
#include "log.h"

#include <string.h>

// FIXME(TS): Just for `_plugin()` funcs
#include "lodge_window.h"
#include "lodge_renderer.h"
#include "env.h"
#include "game.h"
#include "vfs.h"
#include "lodge_plugin_vfs.h"
#include "lodge_plugin_files.h"
#include "lodge_plugin_fbx.h"
#include "lodge_plugin_images.h"
#include "lodge_plugin_textures.h"
#include "lodge_plugin_shaders.h"

#define LODGE_PLUGINS_MAX				128
#define LODGE_PLUGIN_DEPENDENCIES_MAX	32

struct lodge_plugin_meta
{
	struct lodge_plugin					*deps[LODGE_PLUGIN_DEPENDENCIES_MAX];
	size_t								deps_count;
};

struct lodge_plugins
{
	int									running;
	struct lodge_plugin					list[LODGE_PLUGINS_MAX];
	struct lodge_plugin_meta			meta[LODGE_PLUGINS_MAX];
	size_t								offsets[LODGE_PLUGINS_MAX];
	int									count;
	char								*data;
	size_t								data_size;
	float								delta_time_factor;
	struct lodge_plugins_frame_times	frame_times;

	// HACK(TS)
	strview_t							mount_dir;
};

static struct lodge_plugin* lodge_plugins_find_plugin_by_data(struct lodge_plugins *plugins, lodge_plugin_data_t data)
{
	char *cur = plugins->data;
	for(int i = 0; i < plugins->count; i++) {
		struct lodge_plugin *plugin = &plugins->list[i];
		if(data == cur) {
			return plugin;
		}
		cur += plugin->size;
	}
	return NULL;
}

static void lodge_plugins_add_dependency(struct lodge_plugins *plugins, size_t plugin_index, struct lodge_plugin *dependee)
{
	struct lodge_plugin_meta *meta = &plugins->meta[plugin_index];

	for(size_t i = 0, count = meta->deps_count; i < count; i++) {
		if(meta->deps[i] == dependee) {
			return;
		}
	}

	meta->deps[meta->deps_count++] = dependee;
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
	for(int i = plugins->count - 1; i >= 0; i--) {
		struct lodge_plugin *plugin = &plugins->list[i];

		const size_t offset = plugins->offsets[i];
		char *plugin_data = &plugins->data[offset];

		debugf("Plugins", "Freeing plugin `" STRVIEW_PRINTF_FMT "`\n", STRVIEW_PRINTF_ARG(plugin->name));

		if(plugin->free) {
			plugin->free(plugin_data);
		}
	}

	free(plugins->data);
	free(plugins);
}

int lodge_plugins_count(struct lodge_plugins *plugins)
{
	return plugins->count;
}

lodge_plugin_data_t lodge_plugins_depend(struct lodge_plugins *plugins, lodge_plugin_data_t dependee, strview_t name)
{
	ASSERT(plugins->data);

	struct lodge_plugin *dependee_plugin = lodge_plugins_find_plugin_by_data(plugins, dependee);
	if(!dependee_plugin) {
		ASSERT_FAIL("Failed to find dependee");
		return NULL;
	}

	char *cur = plugins->data;
	for(int i = 0; i < plugins->count; i++) {
		struct lodge_plugin *plugin = &plugins->list[i];
		if(strview_equals(plugin->name, name)) {
			lodge_plugins_add_dependency(plugins, i, dependee_plugin);
			return cur;
		}
		cur += plugin->size;
	}

	// TODO(TS): remove `dependee` from all plugins when unloaded

	return NULL;
}

void lodge_plugins_append(struct lodge_plugins *plugins, struct lodge_plugin plugin)
{
	plugins->list[plugins->count++] = plugin;
}

struct lodge_ret lodge_plugins_find(struct lodge_plugins *plugins, strview_t mount_dir)
{
	// TODO(TS): find plugins either by looking for dynamic libraries in filesystem or static list

	// HACK(TS)
	plugins->mount_dir = mount_dir;

	lodge_plugins_append(plugins, lodge_plugin_vfs());
	lodge_plugins_append(plugins, lodge_plugin_windows());
	lodge_plugins_append(plugins, lodge_plugin_env());
	lodge_plugins_append(plugins, lodge_files_plugin());
	lodge_plugins_append(plugins, lodge_fbx_plugin());
	lodge_plugins_append(plugins, lodge_images_plugin());
	lodge_plugins_append(plugins, lodge_textures_plugin());
	lodge_plugins_append(plugins, lodge_shaders_plugin());
	lodge_plugins_append(plugins, game_plugin());

	for(int i = 0; i < plugins->count; i++) {
		debugf("Plugins", "Found plugin: `" STRVIEW_PRINTF_FMT "` (%.1f kB)\n",
			STRVIEW_PRINTF_ARG(plugins->list[i].name),
			plugins->list[i].size / 1024.0f
		);
	}

	return lodge_success();
}

struct lodge_ret lodge_plugins_init(struct lodge_plugins *plugins)
{
	const int count = plugins->count;
	
	/* Preallocate plugins data */
	plugins->data_size = 0;
	for(int i = 0; i < count; i++) {
		struct lodge_plugin *plugin = &plugins->list[i];
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
		struct lodge_plugin *plugin = &plugins->list[i];

		if(plugin->version != LODGE_PLUGIN_VERSION) {
			errorf("Plugins", "Incorrect plugin version: %d (expected: %d)\n", plugin->version, LODGE_PLUGIN_VERSION);
			return lodge_error("Incorrect plugin version");
		}

		memset(cur, 0, plugin->size);

		if(plugin->init) {
			debugf("Plugins", "Initializing `%s`...\n", plugin->name.s);
			struct lodge_ret init_ret = plugin->init(cur, plugins);
			if(!init_ret.success) {
				errorf("Plugins", "Error when initializing plugin `%s`: %s\n", plugin->name.s, init_ret.message.s);
				return init_ret;
			}

			// HACK(TS): Need nice way to mount default dir
			if(strview_equals(plugin->name, strview_static("vfs")))
			{
				vfs_mount((struct vfs*)cur, plugins->mount_dir);
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
	plugins->running = 1;

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
				struct lodge_plugin *plugin = &plugins->list[i];
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
				struct lodge_plugin *plugin = &plugins->list[i];
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

void lodge_plugins_set_running(struct lodge_plugins *plugins, int running)
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
