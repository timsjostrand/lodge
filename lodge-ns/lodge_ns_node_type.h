#ifndef _LODGE_NS_NODE_TYPE_H
#define _LODGE_NS_NODE_TYPE_H

#include "lodge_ns.h"
#include "strview.h"

struct lodge_variant;

typedef bool		(*lodge_node_type_configure_func_t)(struct lodge_node *node);
typedef bool		(*lodge_node_type_run_func_t)(struct lodge_node *node);

lodge_node_type_t	lodge_node_type_register(strview_t name, lodge_node_type_configure_func_t configure_func, lodge_node_type_run_func_t run_func);
//lodge_node_type_t	lodge_node_type_register_default(strview_t name, lodge_node_type_configure_func_t configure_func, lodge_node_type_run_func_t run_func);
lodge_node_type_t	lodge_node_type_find(strview_t name);

bool				lodge_node_type_update_node(lodge_node_type_t node_type, lodge_node_t node);
bool				lodge_node_type_configure_node(lodge_node_type_t node_type, lodge_node_t node);

size_t				lodge_node_type_get_count();
lodge_node_type_t	lodge_node_type_get_index(size_t index);
strview_t			lodge_node_type_get_name(lodge_node_type_t node_type);

void				lodge_node_type_set_default_config(lodge_node_type_t node_type, struct lodge_variant *config);

#endif