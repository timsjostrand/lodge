#include "lodge_type.h"

#include "membuf.h"
#include "strbuf.h"
#include "math4.h"

#include "lodge_assert.h"
#include "lodge_platform.h"
#include "lodge_variant.h"
#include "lodge_properties.h"

#include <stddef.h>
#include <string.h>

struct lodge_type
{
	char					name[255];
	size_t					size;
	void					*funcs[255];
	struct lodge_variant	userdata;
};

struct lodge_type			lodge_types[1024];
size_t						lodge_types_count = 13;
size_t						lodge_types_func_index = 1;

lodge_type_t				LODGE_TYPE_NONE = NULL;
lodge_type_t				LODGE_TYPE_BOOL = &lodge_types[0];
lodge_type_t				LODGE_TYPE_U32 = &lodge_types[1];
lodge_type_t				LODGE_TYPE_U64 = &lodge_types[2];
lodge_type_t				LODGE_TYPE_I32 = &lodge_types[3];
lodge_type_t				LODGE_TYPE_I64 = &lodge_types[4];
lodge_type_t				LODGE_TYPE_F32 = &lodge_types[5];
lodge_type_t				LODGE_TYPE_F64 = &lodge_types[6];
lodge_type_t				LODGE_TYPE_VEC2 = &lodge_types[7];
lodge_type_t				LODGE_TYPE_VEC3 = &lodge_types[8];
lodge_type_t				LODGE_TYPE_VEC4 = &lodge_types[9];
lodge_type_t				LODGE_TYPE_MAT4 = &lodge_types[10];
lodge_type_t				LODGE_TYPE_ENUM_DESC = &lodge_types[11];
lodge_type_t				LODGE_TYPE_PROPERTIES = &lodge_types[12];

static void lodge_types_set(lodge_type_t type, strview_t name, size_t size)
{
	// TODO(TS): assert+skip if already exists
	strbuf_set(strbuf_wrap(type->name), name);
	type->size = size;
}

static void lodge_types_set_index(size_t index, strview_t name, size_t size)
{
	lodge_types_set(&lodge_types[index], name, size);
}

void lodge_types_default_register()
{
	lodge_types_set_index(0,	strview_static("bool"),			sizeof(bool));
	lodge_types_set_index(1,	strview_static("u32"),			sizeof(uint32_t));
	lodge_types_set_index(2,	strview_static("u64"),			sizeof(uint64_t));
	lodge_types_set_index(3,	strview_static("i32"),			sizeof(int32_t));
	lodge_types_set_index(4,	strview_static("i64"),			sizeof(int64_t));
	lodge_types_set_index(5,	strview_static("f32"),			sizeof(float));
	lodge_types_set_index(6,	strview_static("f64"),			sizeof(double));
	lodge_types_set_index(7,	strview_static("vec2"),			sizeof(vec2));
	lodge_types_set_index(8,	strview_static("vec3"),			sizeof(vec3));
	lodge_types_set_index(9,	strview_static("vec4"),			sizeof(vec4));
	lodge_types_set_index(10,	strview_static("mat4"),			sizeof(mat4));
	lodge_types_set_index(11,	strview_static("enum_desc"),	sizeof(struct lodge_enum_desc));
	lodge_types_set_index(12,	strview_static("properties"),	sizeof(struct lodge_properties));
}

size_t lodge_types_make_func_index()
{
	return lodge_types_func_index++;
}

lodge_type_t lodge_type_register(strview_t name, size_t size)
{
	lodge_type_t lodge_type = membuf_append(membuf_wrap(lodge_types), &lodge_types_count, &(struct lodge_type) { 0 }, sizeof(struct lodge_type));
	lodge_types_set(lodge_type, name, size);
	return lodge_type;
}

lodge_type_t lodge_type_find(strview_t name)
{
	for(size_t i = 0; i < lodge_types_count; i++) {
		struct lodge_type *lodge_type = &lodge_types[i];
		if(strview_equals(strbuf_wrap_and(lodge_type->name, strbuf_to_strview), name)) {
			return lodge_type;
		}
	}
	return NULL;
}

strview_t lodge_type_get_name(lodge_type_t type)
{
	return strbuf_wrap_and(type->name, strbuf_to_strview);
}

size_t lodge_type_get_size(lodge_type_t type)
{
	if(!type) {
		return 0;
	}
	return type->size;
}

void* lodge_type_get_func(lodge_type_t type, size_t func_index)
{
	ASSERT(func_index > 0);
	return type->funcs[func_index - 1];
}

void lodge_type_set_func(lodge_type_t type, size_t func_index, void *func)
{
	ASSERT(type);

	if(type) {
		ASSERT(func_index > 0);
		func_index--;
		ASSERT(func_index < LODGE_ARRAYSIZE(type->funcs));
		ASSERT(type->funcs[func_index] == NULL);
		type->funcs[func_index] = func;
	}
}

lodge_type_t lodge_type_register_enum(strview_t name, struct lodge_enum_desc desc)
{
	return lodge_type_register_with_userdata(name, sizeof(enum dummy), LODGE_TYPE_ENUM_DESC, &desc);
}

const struct lodge_enum_desc* lodge_type_get_enum_desc(lodge_type_t type)
{
	return lodge_type_get_userdata(type, LODGE_TYPE_ENUM_DESC);
}

lodge_type_t lodge_type_register_property_object(strview_t name, size_t size, struct lodge_properties *properties)
{
	return lodge_type_register_with_userdata(name, size, LODGE_TYPE_PROPERTIES, properties);
}

struct lodge_properties* lodge_type_get_properties(lodge_type_t type)
{
	return lodge_type_get_userdata(type, LODGE_TYPE_PROPERTIES);
}

lodge_type_t lodge_type_register_with_userdata(strview_t name, size_t size, lodge_type_t userdata_type, void *userdata)
{
	ASSERT_OR(userdata_type && userdata) { return NULL; }
	lodge_type_t type = lodge_type_register(name, size);
	ASSERT_OR(type) { return NULL; }
	lodge_variant_set_type(&type->userdata, userdata_type, userdata);
	return type;
}

void* lodge_type_get_userdata(lodge_type_t type, lodge_type_t userdata_type)
{
	ASSERT_OR(type) { return NULL; }
	ASSERT_OR(userdata_type) { return NULL; }
	if(type) {
		return (void*)lodge_variant_get_type(&type->userdata, userdata_type);
	}
	return NULL;
}
