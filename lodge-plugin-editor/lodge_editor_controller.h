#ifndef _LODGE_EDITOR_CONTROLLER_H
#define _LODGE_EDITOR_CONTROLLER_H

#include "math4.h"

//
// Component
//

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_input;

struct lodge_editor_controller_component 
{
	vec3						target_pos;

	float						move_speed;
	float						move_lerp_speed;
	float						mouse_sensitivity;

	vec2						last_mouse_pos;

	struct lodge_input			*input;
};

lodge_component_type_t			lodge_editor_controller_component_type_register();

//
// System
//

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

lodge_system_type_t				lodge_editor_controller_system_type_register(struct lodge_plugin_editor *plugin);

#endif