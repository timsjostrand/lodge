#ifndef _LODGE_WATER_COMPONENT_H
#define _LODGE_WATER_COMPONENT_H

#include "math4.h"

#include "lodge_texture.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

struct lodge_plugin_water;

struct lodge_water_component
{
	vec3							wave_scale;
	float							max_depth;
	vec3							color;
	lodge_asset_t					foam_asset;
	lodge_asset_t					normals_1_asset;
	lodge_asset_t					normals_2_asset;
};

extern lodge_component_type_t		LODGE_COMPONENT_TYPE_WATER;

lodge_component_type_t				lodge_water_component_type_register(struct lodge_plugin_water *plugin, lodge_type_t texture_asset_type);

#endif