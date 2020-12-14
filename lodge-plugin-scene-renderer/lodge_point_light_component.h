#ifndef _LODGE_POINT_LIGHT_COMPONENT_H
#define _LODGE_POINT_LIGHT_COMPONENT_H

#include "math4.h"

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_point_light_component
{
	vec3						intensities;
	float						attenuation;
	float						ambient_coefficient;
	float						cone_angle;
	vec3						cone_direction;
};

extern lodge_component_type_t	LODGE_COMPONENT_TYPE_POINT_LIGHT;

lodge_component_type_t			lodge_point_light_component_type_register();

#endif