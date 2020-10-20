#include "lodge_serialize_json.h"

#include "str.h"

#include "lodge_variant.h"
#include "lodge_properties.h"
#include "lodge_json.h"
#include "lodge_assert.h"

static bool lodge_json_read_type_func_f32(const lodge_json_t node, float *dst)
{
	double tmp;
	if(!lodge_json_get_number(node, &tmp)) {
		return false;
	}
	*dst = (float)tmp;
	return true;
}

static lodge_json_t lodge_json_write_type_func_f32(const float *src)
{
	return lodge_json_new_number((double)*src);
}

static bool lodge_json_read_type_func_f64(const lodge_json_t node, double *dst)
{
	return lodge_json_get_number(node, dst);
}

static lodge_json_t lodge_json_write_type_func_f64(const double *src)
{
	return lodge_json_new_number(*src);
}

static bool lodge_json_read_type_func_u32(const lodge_json_t node, uint32_t *dst)
{
	double tmp;
	if(!lodge_json_get_number(node, &tmp)) {
		return false;
	}
	*dst = (uint32_t)tmp;
	return true;
}

static lodge_json_t lodge_json_write_type_func_u32(const uint32_t *src)
{
	return lodge_json_new_number((double)*src);
}

static bool lodge_json_read_type_func_u64(const lodge_json_t node, uint64_t *dst)
{
	double tmp;
	if(!lodge_json_get_number(node, &tmp)) {
		return false;
	}
	*dst = (uint64_t)tmp;
	return true;
}

static lodge_json_t lodge_json_write_type_func_u64(const uint64_t *src)
{
	return lodge_json_new_number((double)*src);
}

static lodge_json_t lodge_json_write_type_func_vec2(const vec2 *src)
{
	return lodge_json_new_float_array(src->v, 2);
}

static bool lodge_json_read_type_func_vec2(const lodge_json_t node, vec2 *dst)
{
	const size_t count = lodge_json_object_get_child_count(node);
	ASSERT_OR(count == 2) { 
		return false;
	}
	for(int i=0; i<2; i++) {
		lodge_json_t child = lodge_json_object_get_child_index(node, i);
		if(!child) {
			return false;
		}
		double tmp;
		if(!lodge_json_get_number(child, &tmp)) {
			return false;
		}
		dst->v[i] = tmp;
	}
	return true;
}

static lodge_json_t lodge_json_write_type_func_vec3(const vec3 *src)
{
	return lodge_json_new_float_array(src->v, 3);
}

static bool lodge_json_read_type_func_vec3(const lodge_json_t node, vec3 *dst)
{
	const size_t count = lodge_json_object_get_child_count(node);
	ASSERT_OR(count == 3) { 
		return false;
	}
	for(int i=0; i<3; i++) {
		lodge_json_t child = lodge_json_object_get_child_index(node, i);
		if(!child) {
			return false;
		}
		double tmp;
		if(!lodge_json_get_number(child, &tmp)) {
			return false;
		}
		dst->v[i] = tmp;
	}
	return true;
}

static lodge_json_t lodge_json_write_type_func_vec4(const vec4 *src)
{
	return lodge_json_new_float_array(src->v, 4);
}

static bool lodge_json_read_type_func_vec4(const lodge_json_t node, vec4 *dst)
{
	const size_t count = lodge_json_object_get_child_count(node);
	ASSERT_OR(count == 4) { 
		return false;
	}
	for(int i=0; i<4; i++) {
		lodge_json_t child = lodge_json_object_get_child_index(node, i);
		if(!child) {
			return false;
		}
		double tmp;
		if(!lodge_json_get_number(child, &tmp)) {
			return false;
		}
		dst->v[i] = tmp;
	}
	return true;
}

static bool lodge_json_read_type_func_bool(const lodge_json_t node, bool *dst)
{
	return lodge_json_get_bool(node, dst);
}

static lodge_json_t lodge_json_write_type_func_bool(const bool *src)
{
	return lodge_json_new_bool(*src);
}

static lodge_type_to_json_func_t lodge_type_get_to_json_func(lodge_type_t type)
{
	lodge_type_to_json_func_t write_func = lodge_type_get_func(type, LODGE_TYPE_FUNC_INDEX_TO_JSON);
	ASSERT(write_func);
	return write_func;
}

static lodge_type_from_json_func_t lodge_type_get_from_json_func(lodge_type_t type)
{
	lodge_type_from_json_func_t read_func = lodge_type_get_func(type, LODGE_TYPE_FUNC_INDEX_FROM_JSON);
	ASSERT(read_func);
	return read_func;
}

lodge_json_t lodge_json_object_set_variant(lodge_json_t object, strview_t name, const struct lodge_variant *variant)
{
	lodge_json_t variant_object = lodge_variant_to_json(variant);
	if(!variant_object) {
		return NULL;
	}
	return lodge_json_object_set(object, name, variant_object);
}

bool lodge_variant_from_json(const lodge_json_t node, struct lodge_variant *dst)
{
	strview_t type_name;
	if(!lodge_json_object_get_string(node, strview_static("type"), &type_name)) {
		return false;
	}

	lodge_type_t type = lodge_type_find(type_name);
	if(type == LODGE_TYPE_NONE) {
		return false;
	}

	lodge_type_from_json_func_t read_func = lodge_type_get_from_json_func(type);
	if(read_func && read_func(node, lodge_variant_access_data_ptr(dst))) {
		dst->type = type;
		return true;
	} else {
		return false;
	}
}

lodge_json_t lodge_variant_to_json(const struct lodge_variant *src)
{
	ASSERT(lodge_variant_is_set(src));
	lodge_type_to_json_func_t write_func = lodge_type_get_to_json_func(src->type);
	if(!write_func) {
		return NULL;
	}

	lodge_json_t data_object = write_func(lodge_variant_get_data_ptr(src));
	if(!data_object) {
		return NULL;
	}

	lodge_json_t variant_object = lodge_json_new_object();
	lodge_json_object_set_string(variant_object, strview_static("type"), lodge_type_get_name(src->type));
	lodge_json_object_set(variant_object, strview_static("data"), data_object);
	return variant_object;
}

#if 0
bool lodge_properties_from_json(const lodge_json_t node, struct lodge_properties *dst)
{
	strview_t type_name;
	if(!lodge_json_object_get_string(node, strview_static("type"), &type_name)) {
		return false;
	}

	lodge_type_t type = lodge_type_find(type_name);
	if(type == LODGE_TYPE_NONE) {
		return false;
	}

	lodge_type_from_json_func_t read_func = lodge_type_get_from_json_func(type);
	if(read_func && read_func(node, lodge_variant_access_data_ptr(dst))) {
		dst->type = type;
		return true;
	} else {
		return false;
	}
}
#endif

static lodge_json_t lodge_enum_to_json(const struct lodge_enum_desc *enum_desc, const void *object)
{
	const uint32_t *index = (const uint32_t *)object;
	ASSERT_OR(*index < enum_desc->count) {
		return NULL;
	}
	strview_t name = enum_desc->elements[*index].name;
	return lodge_json_new_string(name);
}

static bool lodge_enum_desc_find_value(const struct lodge_enum_desc *enum_desc, strview_t name, int *value_out)
{
	for(size_t i = 0; i < enum_desc->count; i++) {
		const struct lodge_enum_value_desc *value_desc = &enum_desc->elements[i];
		if(strview_equals(value_desc->name, name)) {
			*value_out = value_desc->value;
			return true;
		}
	}
	return false;
}

static bool lodge_enum_from_json(lodge_json_t node, const struct lodge_enum_desc *enum_desc, void *object)
{
	strview_t name;
	if(!lodge_json_get_string(node, &name)) {
		return false;
	}

	int tmp = 0;
	if(!lodge_enum_desc_find_value(enum_desc, name, &tmp)) {
		return false;
	}

	*(int*)(object) = tmp;
	return true;
}

lodge_json_t lodge_property_to_json(const struct lodge_property *property, const void *object)
{
	ASSERT_OR(property && object) {
		return NULL;
	}

	const void *property_data = lodge_property_get(property, object);
	ASSERT_OR(property_data) {
		return NULL;
	}

	//
	// Enum desc object?
	//
	const struct lodge_enum_desc *enum_desc = lodge_type_get_enum_desc(property->type);
	if(enum_desc) {
		return lodge_enum_to_json(enum_desc, property_data);
	}

	//
	// Recursive property object?
	//
	struct lodge_properties *properties = lodge_type_get_properties(property->type);
	if(properties) {
		return lodge_properties_to_json(properties, property_data);
	}

	lodge_type_to_json_func_t to_json_func = lodge_type_get_to_json_func(property->type);
	ASSERT_OR(to_json_func) {
		return NULL;
	}

	return to_json_func(property_data);
}

lodge_json_t lodge_properties_to_json(const struct lodge_properties *properties, const void *object)
{
	ASSERT_OR(properties && object) {
		return NULL;
	}

	lodge_json_t root = lodge_json_new_object();

	for(size_t i = 0, count = properties->count; i < count; i++) {
		const struct lodge_property *property = &properties->elements[i];

		if(LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_TRANSIENT)) {
			continue;
		}

		lodge_json_t property_object = lodge_property_to_json(property, object);

		// FIXME(TS): error handling: clean up and return null
		ASSERT(property_object);

		lodge_json_object_set(root, property->name, property_object);
	}

	return root;
}

bool lodge_property_from_json(const lodge_json_t node, struct lodge_property *property, void *object)
{
	ASSERT_OR(node && property && object) {
		return false;
	}

	// FIXME(TS): const-cast
	void *property_data = (void*)lodge_property_get(property, object);
	ASSERT_OR(property_data) {
		return false;
	}

	//
	// Enum desc object?
	//
	const struct lodge_enum_desc *enum_desc = lodge_type_get_enum_desc(property->type);
	if(enum_desc) {
		return lodge_enum_from_json(node, enum_desc, property_data);
	}

	//
	// Recursive property object?
	//
	struct lodge_properties *properties = lodge_type_get_properties(property->type);
	if(properties) {
		return lodge_properties_from_json(node, properties, property_data);
	}

	lodge_type_from_json_func_t from_json_func = lodge_type_get_from_json_func(property->type);
	ASSERT_OR(from_json_func) {
		return false;
	}

	if(!from_json_func(node, property_data)) {
		return false;
	}

	lodge_property_set(property, object, property_data);
	return true;
}

bool lodge_properties_from_json(const lodge_json_t node, struct lodge_properties *properties, void *object)
{
	ASSERT_OR(node && properties && object) {
		return false;
	}

	for(size_t i = 0, count = properties->count; i < count; i++) {
		struct lodge_property *property = &properties->elements[i];

		if(LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_TRANSIENT)) {
			continue;
		}

		lodge_json_t property_object = lodge_json_object_get_child(node, property->name);
		ASSERT_OR(property_object) {
			return false;
		}

		if(!lodge_property_from_json(property_object, property, object)) {
			return false;
		}
	}

	return true;
}

size_t LODGE_TYPE_FUNC_INDEX_TO_JSON = 0;
size_t LODGE_TYPE_FUNC_INDEX_FROM_JSON = 0;

void lodge_json_register_func_indices()
{
	ASSERT(LODGE_TYPE_FUNC_INDEX_TO_JSON == 0);
	ASSERT(LODGE_TYPE_FUNC_INDEX_FROM_JSON == 0);

	LODGE_TYPE_FUNC_INDEX_TO_JSON = lodge_types_make_func_index();
	LODGE_TYPE_FUNC_INDEX_FROM_JSON = lodge_types_make_func_index();

	lodge_json_register_type_funcs(LODGE_TYPE_F32, &lodge_json_write_type_func_f32, &lodge_json_read_type_func_f32);
	lodge_json_register_type_funcs(LODGE_TYPE_F64, &lodge_json_write_type_func_f64, &lodge_json_read_type_func_f64);
	lodge_json_register_type_funcs(LODGE_TYPE_U32, &lodge_json_write_type_func_u32, &lodge_json_read_type_func_u32);
	lodge_json_register_type_funcs(LODGE_TYPE_U64, &lodge_json_write_type_func_u64, &lodge_json_read_type_func_u64);
	lodge_json_register_type_funcs(LODGE_TYPE_VEC2, &lodge_json_write_type_func_vec2, &lodge_json_read_type_func_vec2);
	lodge_json_register_type_funcs(LODGE_TYPE_VEC3, &lodge_json_write_type_func_vec3, &lodge_json_read_type_func_vec3);
	lodge_json_register_type_funcs(LODGE_TYPE_VEC4, &lodge_json_write_type_func_vec4, &lodge_json_read_type_func_vec4);
	lodge_json_register_type_funcs(LODGE_TYPE_BOOL, &lodge_json_write_type_func_bool, &lodge_json_read_type_func_bool);
}

void lodge_json_register_type_funcs(lodge_type_t type, lodge_type_to_json_func_t write_func, lodge_type_from_json_func_t read_func)
{
	ASSERT(LODGE_TYPE_FUNC_INDEX_TO_JSON > 0);
	ASSERT(LODGE_TYPE_FUNC_INDEX_FROM_JSON > 0);
	lodge_type_set_func(type, LODGE_TYPE_FUNC_INDEX_TO_JSON, write_func);
	lodge_type_set_func(type, LODGE_TYPE_FUNC_INDEX_FROM_JSON, read_func);
}
