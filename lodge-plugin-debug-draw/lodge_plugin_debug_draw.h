#ifndef _LODGE_DEBUG_DRAW_COMPONENTS_H
#define _LODGE_DEBUG_DRAW_COMPONENTS_H

#include "lodge_plugin.h"

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_plugin_debug_draw;

extern lodge_component_type_t	LODGE_COMPONENT_TYPE_DEBUG_SPHERE;

struct lodge_plugin_desc		lodge_plugin_debug_draw();
lodge_system_type_t				lodge_plugin_debug_draw_get_system_type(struct lodge_plugin_debug_draw *plugin);
lodge_component_type_t			lodge_plugin_debug_draw_get_sphere_component_type(struct lodge_plugin_debug_draw *plugin);

//lodge_component_type_t		lodge_debug_sphere_component_type_register();
//lodge_system_type_t			lodge_debug_draw_system_type_register();

struct lodge_debug_draw*		lodge_debug_draw_system_get_batcher(struct lodge_debug_draw_system *system);

#endif