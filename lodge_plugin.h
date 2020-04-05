#ifndef _LODGE_PLUGIN_H
#define _LODGE_PLUGIN_H

#include "lodge.h"

#define LODGE_PLUGIN_VERSION 1

struct lodge_plugins;

typedef struct lodge_ret		(*lodge_plugin_init_func_t)(void *plugin, struct lodge_plugins *plugins);
typedef void					(*lodge_plugin_update_func_t)(void *plugin, float delta_time);
typedef void					(*lodge_plugin_render_func_t)(void *plugin);
typedef void					(*lodge_plugin_free_func_t)(void *plugin);

struct lodge_plugin
{
	uint32_t					version;
	size_t						size;
	strview_t					name;
	lodge_plugin_init_func_t	init;
	lodge_plugin_update_func_t	update;
	lodge_plugin_render_func_t	render; // TODO(TS): remove
	lodge_plugin_free_func_t	free;
};

typedef struct lodge_plugin		(*lodge_plugin_func_t)();

#endif