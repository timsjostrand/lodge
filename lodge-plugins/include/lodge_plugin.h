#ifndef _LODGE_PLUGIN_H
#define _LODGE_PLUGIN_H

#include "lodge.h"
#include "lodge_static_call.h"

#define LODGE_PLUGIN_VERSION 1

struct lodge_plugins;
struct lodge_argv;

typedef struct lodge_ret				(*lodge_plugin_new_inplace_func_t)(void *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args);
typedef void							(*lodge_plugin_update_func_t)(void *plugin, float delta_time);
typedef void							(*lodge_plugin_render_func_t)(void *plugin);
typedef void							(*lodge_plugin_free_inplace_func_t)(void *plugin);

struct lodge_static_mount
{
	strview_t							src_dir;
	strview_t							dst_point;
};

struct lodge_static_mounts
{
	size_t								count;
	struct lodge_static_mount			elements[8];
};

struct lodge_plugin_desc
{
	uint32_t							version;
	size_t								size;
	strview_t							name;
	lodge_plugin_new_inplace_func_t		new_inplace;
	lodge_plugin_free_inplace_func_t	free_inplace;
	lodge_plugin_update_func_t			update;
	lodge_plugin_render_func_t			render; // TODO(TS): remove
	struct lodge_static_mounts			static_mounts;
};

typedef struct lodge_plugin_desc		(*lodge_plugin_func_t)();

#define LODGE_PLUGIN_DECL(name) \
	void name ## _register(); \
	struct lodge_plugin_desc name()

#define LODGE_PLUGIN_IMPL(name) \
	struct lodge_plugin_desc name()

extern lodge_plugin_func_t				*lodge_plugin_registry;
extern size_t							lodge_plugin_registry_count;

void									lodge_plugin_registry_append(lodge_plugin_func_t plugin_func);


#endif