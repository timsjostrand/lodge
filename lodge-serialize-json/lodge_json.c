#include "lodge_json.h"

#include "str.h"
#include "lodge_assert.h"

#include "cjson/cJSON.h"

lodge_json_t lodge_json_new_object()
{
	return cJSON_CreateObject();
}

lodge_json_t lodge_json_new_array()
{
	return cJSON_CreateArray();
}

lodge_json_t lodge_json_new_float_array(const float *data, size_t count)
{
	return cJSON_CreateFloatArray(data, count);
}

lodge_json_t lodge_json_new_number(double value)
{
	return cJSON_CreateNumber(value);
}

lodge_json_t lodge_json_new_bool(bool value)
{
	return cJSON_CreateBool(value);
}

lodge_json_t lodge_json_new_string(strview_t value)
{
	ASSERT_NULL_TERMINATED(value);
	return cJSON_CreateString(value.s);
}

void lodge_json_free(lodge_json_t root)
{
	cJSON_Delete(root);
}

lodge_json_t lodge_json_object_set_new_array(lodge_json_t object, strview_t name)
{
	ASSERT_NULL_TERMINATED(name);
	ASSERT(cJSON_IsObject(object));
	return cJSON_AddArrayToObject(object, name.s);
}

lodge_json_t lodge_json_object_set_new_object(lodge_json_t object, strview_t name)
{
	ASSERT_NULL_TERMINATED(name);
	ASSERT(cJSON_IsObject(object));
	return cJSON_AddObjectToObject(object, name.s);
}

lodge_json_t lodge_json_object_set_number(lodge_json_t object, strview_t name, double value)
{
	ASSERT_NULL_TERMINATED(name);
	ASSERT(cJSON_IsObject(object));
	return cJSON_AddNumberToObject(object, name.s, value);
}

lodge_json_t lodge_json_object_set_string(lodge_json_t object, strview_t name, strview_t value)
{
	ASSERT_NULL_TERMINATED(name);
	ASSERT_NULL_TERMINATED(value);
	ASSERT(cJSON_IsObject(object));
	return cJSON_AddStringToObject(object, name.s, value.s);
}

lodge_json_t lodge_json_object_set(lodge_json_t object, strview_t name, lodge_json_t child)
{
	ASSERT_NULL_TERMINATED(name);
	ASSERT(cJSON_IsObject(object));
	cJSON_AddItemToObject(object, name.s, child);
	return child;
}

#if 0
lodge_json_t lodge_json_set_variant(lodge_json_t object, strview_t name, const struct lodge_variant *variant)
{
	ASSERT_NULL_TERMINATED(name);
}
#endif

lodge_json_t lodge_json_array_append_new_object(lodge_json_t arr)
{
	ASSERT(cJSON_IsArray(arr));
	lodge_json_t new_object = lodge_json_new_object();
	cJSON_AddItemToArray(arr, new_object);
	return new_object;
}

lodge_json_t lodge_json_array_append_number(lodge_json_t arr, double value)
{
	ASSERT(cJSON_IsArray(arr));
	lodge_json_t new_object = lodge_json_new_number(value);
	cJSON_AddItemToArray(arr, new_object);
	return new_object;
}

char* lodge_json_to_string(lodge_json_t object)
{
	// FIXME(TS): use one of the buffered/preallocated alternatives instead
	return cJSON_Print(object);
}

lodge_json_t lodge_json_from_string(strview_t str)
{
	ASSERT_NULL_TERMINATED(str);
	return cJSON_Parse(str.s);
}

lodge_json_t lodge_json_object_get_array(lodge_json_t object, strview_t name)
{
	ASSERT_NULL_TERMINATED(name);
	lodge_json_t arr = cJSON_GetObjectItemCaseSensitive(object, name.s);
	return cJSON_IsArray(arr) ? arr : NULL;
}

bool lodge_json_object_get_string(lodge_json_t object, strview_t name, strview_t *value_out)
{
	ASSERT_NULL_TERMINATED(name);
	return lodge_json_get_string(cJSON_GetObjectItemCaseSensitive(object, name.s), value_out);
}

bool lodge_json_object_get_number(lodge_json_t object, strview_t name, double *value_out)
{
	ASSERT_NULL_TERMINATED(name);
	return lodge_json_get_number(cJSON_GetObjectItemCaseSensitive(object, name.s), value_out);
}

size_t lodge_json_object_get_child_count(lodge_json_t object)
{
	return (size_t)cJSON_GetArraySize(object);
}

lodge_json_t lodge_json_object_get_child_index(lodge_json_t object, size_t index)
{
	return cJSON_GetArrayItem(object, index);
}

bool lodge_json_object_get_name(lodge_json_t object, strview_t *name_out)
{
	if(cJSON_IsObject(object) && object->string) {
		*name_out = strview_make(object->string, strlen(object->string));
		return true;
	}
	return false;
}

lodge_json_t lodge_json_object_get_child(lodge_json_t object, strview_t name)
{
	ASSERT_NULL_TERMINATED(name);
	return cJSON_GetObjectItemCaseSensitive(object, name.s);
}

bool lodge_json_get_string(lodge_json_t object, strview_t *value_out)
{
	// FIXME(TS): cJSON_IsRaw() also supported?
	if(!object || !cJSON_IsString(object)) {
		return false;
	}
	*value_out = strview_make(object->valuestring, strlen(object->valuestring));
	return true;
}

bool lodge_json_get_number(lodge_json_t object, double *value_out)
{
	if(!object || !cJSON_IsNumber(object)) {
		return false;
	}
	*value_out = object->valuedouble;
	return true;
}

bool lodge_json_get_bool(lodge_json_t object, bool *value_out)
{
	if(!object || !cJSON_IsBool(object)) {
		return false;
	}
	*value_out = object->type == cJSON_True;
	return true;
}
