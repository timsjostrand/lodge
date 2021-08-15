#ifndef _LODGE_TYPE_H
#define _LODGE_TYPE_H

#include "strview.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

extern lodge_type_t					LODGE_TYPE_NONE;
extern lodge_type_t					LODGE_TYPE_BOOL;
extern lodge_type_t					LODGE_TYPE_U32;
extern lodge_type_t					LODGE_TYPE_U64;
extern lodge_type_t					LODGE_TYPE_I32;
extern lodge_type_t					LODGE_TYPE_I64;
extern lodge_type_t					LODGE_TYPE_F32;
extern lodge_type_t					LODGE_TYPE_F64;
extern lodge_type_t					LODGE_TYPE_VEC2;
extern lodge_type_t					LODGE_TYPE_VEC3;
#define								LODGE_TYPE_VEC3_COLOR LODGE_TYPE_VEC3
extern lodge_type_t					LODGE_TYPE_VEC4;
#define								LODGE_TYPE_VEC4_COLOR LODGE_TYPE_VEC4
extern lodge_type_t					LODGE_TYPE_MAT4;

extern lodge_type_t					LODGE_TYPE_ENUM_DESC;

void								lodge_types_default_register();
size_t								lodge_types_make_func_index();

lodge_type_t						lodge_type_register(strview_t name, size_t size);
lodge_type_t						lodge_type_find(strview_t name);

strview_t							lodge_type_get_name(lodge_type_t type);
size_t								lodge_type_get_size(lodge_type_t type);
void*								lodge_type_get_func(lodge_type_t type, size_t func_index);

void								lodge_type_set_func(lodge_type_t type, size_t func_index, void *func);

//
// Enum helpers
//

struct lodge_enum_value_desc
{
	strview_t						name;
	int								value;
};

struct lodge_enum_desc
{
	size_t							count;
	struct lodge_enum_value_desc	elements[256];
};

lodge_type_t						lodge_type_register_enum(strview_t name, struct lodge_enum_desc desc);
const struct lodge_enum_desc*		lodge_type_get_enum_desc(lodge_type_t type);

#endif