#ifndef _LODGE_SERIALIZE_JSON_H
#define _LODGE_SERIALIZE_JSON_H

#include "lodge_type.h"

#include <stdbool.h>

struct cJSON;
typedef struct cJSON* lodge_json_t;

struct lodge_variant;
struct lodge_property;
struct lodge_properties;

typedef bool				(*lodge_type_from_json_func_t)(const lodge_json_t node, void *dst);
typedef lodge_json_t		(*lodge_type_to_json_func_t)(const void *src);

lodge_json_t				lodge_json_object_set_variant(lodge_json_t object, strview_t name, const struct lodge_variant *variant);

void						lodge_json_register_func_indices();
void						lodge_json_register_type_funcs(lodge_type_t type, lodge_type_to_json_func_t write_func, lodge_type_from_json_func_t read_func);

lodge_json_t				lodge_variant_to_json(const struct lodge_variant *src);
bool						lodge_variant_from_json(const lodge_json_t node, struct lodge_variant *dst);

lodge_json_t				lodge_property_to_json(const struct lodge_property *property, const void *object);
lodge_json_t				lodge_properties_to_json(const struct lodge_properties *properties, const void *object);

bool						lodge_property_from_json(const lodge_json_t node, struct lodge_property *property, void *object);
bool						lodge_properties_from_json(const lodge_json_t node, struct lodge_properties *properties, void *object);

extern size_t				LODGE_TYPE_FUNC_INDEX_TO_JSON;
extern size_t				LODGE_TYPE_FUNC_INDEX_FROM_JSON;

#endif