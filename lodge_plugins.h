#ifndef _LODGE_PLUGINS_H
#define _LODGE_PLUGINS_H

// TODO(TS): rename `lodge_plugins` => `lodge_core`

#include "str.h"

struct lodge_plugin;
struct lodge_plugins;

typedef void* lodge_plugin_data_t;

struct lodge_plugins_frame_times {
	int			frames;						/* Number of frames drawn since last_frame_report. */
	double		last_frame_report;			/* When frames were last summed up. */
	double		last_frame;					/* When the last frame was drawn. */
	float		frame_time_min;
	float		frame_time_max;
	float		frame_time_sum;
	float		frame_time_avg;
};

struct lodge_plugins*				lodge_plugins_new();
struct lodge_ret					lodge_plugins_find(struct lodge_plugins *plugins);
void								lodge_plugins_free(struct lodge_plugins *plugins);

lodge_plugin_data_t					lodge_plugins_depend(struct lodge_plugins *plugins, lodge_plugin_data_t dependee, strview_t name);
struct lodge_ret					lodge_plugins_init(struct lodge_plugins *plugins);
int									lodge_plugins_count(struct lodge_plugins *plugins);
void								lodge_plugins_run(struct lodge_plugins *plugins);

void								lodge_plugins_set_running(struct lodge_plugins *plugins, int running);
void								lodge_plugins_set_delta_time_factor(struct lodge_plugins *plugins, float delta_time_factor);
struct lodge_plugins_frame_times	lodge_plugins_get_frame_times(struct lodge_plugins *plugins);

#endif