#ifndef _LODGE_PLUGIN_DEBUG_DRAW_H
#define _LODGE_PLUGIN_DEBUG_DRAW_H

#include "lodge_plugin.h"

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_plugin_debug_draw;

struct lodge_debug_draw_system;

extern lodge_component_type_t	LODGE_COMPONENT_TYPE_DEBUG_SPHERE;

lodge_system_type_t				lodge_plugin_debug_draw_get_system_type(struct lodge_plugin_debug_draw *plugin);
lodge_component_type_t			lodge_plugin_debug_draw_get_sphere_component_type(struct lodge_plugin_debug_draw *plugin);

struct lodge_debug_draw*		lodge_debug_draw_system_get_batcher(struct lodge_debug_draw_system *system);

LODGE_PLUGIN_DECL(lodge_plugin_debug_draw);

#endif