#ifndef _LODGE_BILLBOARD_COMPONENT_COMPONENT_H
#define _LODGE_BILLBOARD_COMPONENT_COMPONENT_H

#include "math4.h"

#include <stdbool.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

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
	lodge_texture_t				texture;
};

lodge_component_type_t			lodge_billboard_component_type_register();

#endif