#ifndef _LODGE_PLUGIN_TERRAIN_H
#define _LODGE_PLUGIN_TERRAIN_H

#include "lodge_plugin.h"

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_plugin_debug_draw;

struct lodge_assets;

struct lodge_plugin_terrain
{
	struct lodge_assets				*shaders;
	void							*plugin_scene_renderer;
	struct lodge_plugin_debug_draw	*plugin_debug_draw;
	lodge_system_type_t				system_type;
	lodge_component_type_t			component_type;
	lodge_component_type_t			foliage_component_type;
};

struct lodge_plugin_desc	lodge_plugin_terrain();
lodge_system_type_t			lodge_plugin_terrain_get_system_type(struct lodge_plugin_terrain *terrain);
lodge_component_type_t		lodge_plugin_terrain_get_component_type(struct lodge_plugin_terrain *terrain);
lodge_component_type_t		lodge_plugin_terrain_get_foliage_component_type(struct lodge_plugin_terrain *terrain);

#endif