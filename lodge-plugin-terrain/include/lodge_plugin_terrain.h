#ifndef _LODGE_PLUGIN_TERRAIN_H
#define _LODGE_PLUGIN_TERRAIN_H

#include "lodge_plugin.h"

struct lodge_plugin_debug_draw;
struct lodge_assets2;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_terrain_types
{
	lodge_system_type_t				terrain_system_type;
	lodge_component_type_t			terrain_component_type;
	lodge_component_type_t			foliage_component_type;
};

//
// TODO(TS): private this struct
//
struct lodge_plugin_terrain
{
	struct lodge_assets2			*shaders;
	struct lodge_assets2			*textures;
	void							*plugin_scene_renderer;
	struct lodge_plugin_debug_draw	*plugin_debug_draw;

	struct lodge_terrain_types		types;
};

struct lodge_plugin_desc			lodge_plugin_terrain();
struct lodge_terrain_types			lodge_plugin_terrain_get_types(struct lodge_plugin_terrain *terrain);

#endif