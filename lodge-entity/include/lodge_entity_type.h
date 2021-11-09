#ifndef _LODGE_ENTITY_TYPE_H
#define _LODGE_ENTITY_TYPE_H

#include "strview.h"

#include <stddef.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_entity_components_desc
{
	size_t									count;
	lodge_component_type_t					elements[16];
};

struct lodge_entity_type_desc
{
	strview_t								name;
	struct lodge_entity_components_desc		components;
};

typedef struct lodge_entity_type* lodge_entity_type_t;

lodge_entity_type_t							lodge_entity_types_get_index(size_t index);
size_t										lodge_entity_types_get_count();

lodge_entity_type_t							lodge_entity_type_register(struct lodge_entity_type_desc desc);

strview_t									lodge_entity_type_get_name(lodge_entity_type_t entity_type);
const struct lodge_entity_components_desc*	lodge_entity_type_get_components(lodge_entity_type_t entity_type);

#endif