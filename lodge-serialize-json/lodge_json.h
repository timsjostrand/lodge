#ifndef _LODGE_JSON_H
#define _LODGE_JSON_H

#include "strview.h"

#include <stdbool.h>

struct cJSON;
typedef struct cJSON* lodge_json_t;

lodge_json_t				lodge_json_new_object();
lodge_json_t				lodge_json_new_array();
lodge_json_t				lodge_json_new_float_array(const float *data, size_t count);
lodge_json_t				lodge_json_new_number(double value);
lodge_json_t				lodge_json_new_string(strview_t value);
lodge_json_t				lodge_json_new_bool(bool value);
void						lodge_json_free(lodge_json_t root);

lodge_json_t				lodge_json_object_set(lodge_json_t object, strview_t name, lodge_json_t child);
lodge_json_t				lodge_json_object_set_new_array(lodge_json_t object, strview_t name);
lodge_json_t				lodge_json_object_set_new_object(lodge_json_t object, strview_t name);
lodge_json_t				lodge_json_object_set_number(lodge_json_t object, strview_t name, double value);
lodge_json_t				lodge_json_object_set_string(lodge_json_t object, strview_t name, strview_t value);

size_t						lodge_json_object_get_child_count(lodge_json_t object);
lodge_json_t				lodge_json_object_get_child_index(lodge_json_t object, size_t index);
lodge_json_t				lodge_json_object_get_child(lodge_json_t object, strview_t name);
bool						lodge_json_object_get_name(lodge_json_t object, strview_t *name_out);

lodge_json_t				lodge_json_object_get_array(lodge_json_t object, strview_t name);
bool						lodge_json_object_get_string(lodge_json_t object, strview_t name, strview_t *value_out);
bool						lodge_json_object_get_number(lodge_json_t object, strview_t name, double *value_out);

lodge_json_t				lodge_json_array_append_new_object(lodge_json_t arr);
lodge_json_t				lodge_json_array_append_number(lodge_json_t arr, double value);

bool						lodge_json_get_string(lodge_json_t object, strview_t *value_out);
bool						lodge_json_get_number(lodge_json_t object, double *value_out);
bool						lodge_json_get_bool(lodge_json_t object, bool *value_out);

char*						lodge_json_to_string(lodge_json_t object);
lodge_json_t				lodge_json_from_string(strview_t str);

#endif