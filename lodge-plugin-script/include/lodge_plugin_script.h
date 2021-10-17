#ifndef _LODGE_PLUGIN_SCRIPT_H
#define _LODGE_PLUGIN_SCRIPT_H

#include "lodge_plugin.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_script_types
{
	lodge_type_t					graph_asset_type;
	lodge_system_type_t				script_system_type;
	lodge_component_type_t			script_component_type;
};

struct lodge_plugin_desc			lodge_plugin_script();
struct lodge_script_types			lodge_plugin_script_get_types(struct lodge_plugin_script *plugin);

#endif