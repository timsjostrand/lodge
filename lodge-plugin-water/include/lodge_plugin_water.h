#ifndef _LODGE_PLUGIN_WATER_H
#define _LODGE_PLUGIN_WATER_H

#include "lodge_plugin.h"

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_assets;

struct lodge_plugin_water
{
	struct lodge_assets		*shaders;
	struct lodge_assets		*textures;
	struct lodge_assets		*images;
	void					*scene_renderer;
	lodge_system_type_t		system_type;
	lodge_component_type_t	component_type;
};

struct lodge_plugin_desc	lodge_plugin_water();
lodge_system_type_t			lodge_plugin_water_get_system_type(struct lodge_plugin_water *plugin);
lodge_component_type_t		lodge_plugin_water_get_component_type(struct lodge_plugin_water *plugin);

#endif