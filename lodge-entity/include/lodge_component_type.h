#ifndef _LODGE_COMPONENT_H
#define _LODGE_COMPONENT_H

#include "strview.h"

#include "lodge_properties.h"
#include "lodge_type.h"

#include <stdint.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

typedef void							(*lodge_component_new_inplace_func_t)(void *component, void *userdata);
typedef void							(*lodge_component_free_inplace_func_t)(void *component, void *userdata);

struct lodge_component_desc
{
	strview_t							name;
	strview_t							description;
	size_t								size;
	lodge_component_new_inplace_func_t	new_inplace;
	lodge_component_free_inplace_func_t	free_inplace;
	struct lodge_properties				properties;
	void								*userdata;
};

lodge_component_type_t					lodge_component_types_get_index(size_t index);
size_t									lodge_component_types_get_count();
lodge_component_type_t					lodge_component_types_find(strview_t name);

lodge_component_type_t					lodge_component_type_register(struct lodge_component_desc desc);

strview_t								lodge_component_type_get_name(lodge_component_type_t component_type);
strview_t								lodge_component_type_get_description(lodge_component_type_t component_type);
size_t									lodge_component_type_get_size(lodge_component_type_t component_type);
struct lodge_properties*				lodge_component_type_get_properties(lodge_component_type_t type);
void									lodge_component_type_new_inplace(lodge_component_type_t component_type, void *dst);
void									lodge_component_type_free_inplace(lodge_component_type_t component_type, void *dst);
lodge_type_t							lodge_component_type_get_type(lodge_component_type_t component_type);

#endif