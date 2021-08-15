#ifndef _LODGE_PLUGIN_EDITOR_H
#define _LODGE_PLUGIN_EDITOR_H

#include "lodge_plugin.h"

struct lodge_plugin_editor;

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;


struct lodge_editor_types
{
	lodge_system_type_t		selection_system_type;

	lodge_system_type_t		controller_system_type;
	lodge_component_type_t	controller_component_type;
};

struct lodge_plugin_desc	lodge_plugin_editor();

struct lodge_editor_types	lodge_plugin_editor_get_types(struct lodge_plugin_editor *editor);

#endif
