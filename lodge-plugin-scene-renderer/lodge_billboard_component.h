#ifndef _LODGE_BILLBOARD_COMPONENT_COMPONENT_H
#define _LODGE_BILLBOARD_COMPONENT_COMPONENT_H

#include "math4.h"

#include <stdbool.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_billboard_component
{
	//
	// The size of the billboard -- should this be scale instead?
	//
	vec2						size;
	
	//
	// Size is kept fixed in screen space.
	//
	bool						fixed_size;
	
	//
	// Hidden when playing or simulating.
	//
	bool						editor_only;

	//
	// The texture to draw.
	//
	lodge_asset_t				texture_asset;
};

lodge_component_type_t			lodge_billboard_component_type_register(lodge_type_t texture_asset_type);

#endif