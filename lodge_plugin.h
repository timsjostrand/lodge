#ifndef _LODGE_PLUGIN_H
#define _LODGE_PLUGIN_H

#include "lodge.h"

#define LODGE_PLUGIN_VERSION 1

struct lodge_plugins;

typedef struct lodge_ret		(*lodge_plugin_init_func)(void *plugin, struct lodge_plugins *plugins);
typedef void					(*lodge_plugin_update_func)(void *plugin, float delta_time);
typedef void					(*lodge_plugin_render_func)(void *plugin);
typedef void					(*lodge_plugin_free_func)(void *plugin);

struct lodge_plugin
{
	uint32_t					version;
	size_t						size;
	strview_t					name;
	lodge_plugin_init_func		init;
	lodge_plugin_update_func	update;
	lodge_plugin_render_func	render;
	lodge_plugin_free_func		free;
};

typedef struct lodge_plugin		(*lodge_plugin_func)();

#endif