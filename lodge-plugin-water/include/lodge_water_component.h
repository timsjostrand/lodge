#ifndef _LODGE_WATER_COMPONENT_H
#define _LODGE_WATER_COMPONENT_H

#include "math4.h"

#include "lodge_texture.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_water_component
{
	vec3							wave_scale;
	float							max_depth;
	vec3							color;
};

extern lodge_component_type_t		LODGE_COMPONENT_TYPE_WATER;

lodge_component_type_t				lodge_water_component_type_register();

#endif