#ifndef _LODGE_DIRECTIONAL_LIGHT_COMPONENT_H
#define _LODGE_DIRECTIONAL_LIGHT_COMPONENT_H

#include "math4.h"

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_directional_light_component
{
	vec3						dir;
	vec3						intensities;
};

extern lodge_component_type_t	LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT;

lodge_component_type_t			lodge_directional_light_component_type_register();


//
// FIXME(TS): This is a hack until multiple directional lights are supported
//

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

vec3							lodge_directional_light_get_dir(lodge_scene_t scene);

#endif