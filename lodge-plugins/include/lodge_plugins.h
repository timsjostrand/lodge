#ifndef _LODGE_PLUGINS_H
#define _LODGE_PLUGINS_H

#include "str.h"

#include "lodge_time.h"
#include "lodge_platform.h"

struct lodge_argv;
struct lodge_plugin_desc;
struct lodge_plugins;

struct lodge_plugins_frame_times
{
	int								frames;						/* Number of frames drawn since last_frame_report. */
	lodge_timestamp_t				last_frame_report;			/* When frames were last summed up. */
	lodge_timestamp_t				last_frame;					/* When the last frame was drawn. */
	float							frame_time_min;
	float							frame_time_max;
	float							frame_time_sum;
	float							frame_time_avg;
};

struct lodge_plugins*				lodge_plugins_new();
struct lodge_ret					lodge_plugins_find(struct lodge_plugins *plugins, const struct lodge_argv *args);
void								lodge_plugins_free(struct lodge_plugins *plugins);

void*								lodge_plugins_depend(struct lodge_plugins *plugins, void *dependee, strview_t name);
bool								lodge_plugins_undepend(struct lodge_plugins *plugins, void *dependee, strview_t name);
bool								lodge_plugins_is_dependency(struct lodge_plugins *plugins, void *dependee, strview_t name);

struct lodge_ret					lodge_plugins_init(struct lodge_plugins *plugins);
void								lodge_plugins_run(struct lodge_plugins *plugins);

void								lodge_plugins_set_running(struct lodge_plugins *plugins, bool running);
void								lodge_plugins_set_delta_time_factor(struct lodge_plugins *plugins, float delta_time_factor);
struct lodge_plugins_frame_times	lodge_plugins_get_frame_times(struct lodge_plugins *plugins);

uint32_t							lodge_plugins_get_count(const struct lodge_plugins *plugins);
const struct lodge_plugin_desc*		lodge_plugins_get_desc(const struct lodge_plugins *plugins, size_t index);
uint32_t							lodge_plugins_get_dependencies_count(const struct lodge_plugins *plugins, size_t index);

#endif