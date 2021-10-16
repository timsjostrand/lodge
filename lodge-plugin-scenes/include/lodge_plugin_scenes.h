#ifndef _LODGE_PLUGIN_SCENES_H
#define _LODGE_PLUGIN_SCENES_H

#include "lodge_plugin.h"

struct lodge_assets2;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_plugin_scenes_types
{
	lodge_type_t					scene_asset_type;
	lodge_component_type_t			transform_component_type;
};

struct lodge_plugin_desc			lodge_plugin_scenes();

struct lodge_plugin_scenes_types	lodge_plugin_scenes_get_asset_type(struct lodge_assets2 *scenes);

#endif