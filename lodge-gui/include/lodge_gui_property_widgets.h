#ifndef _LODGE_GUI_PROPERTY_WIDGET_FACTORY_H
#define _LODGE_GUI_PROPERTY_WIDGET_FACTORY_H

#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_property;
struct lodge_properties;
struct nk_context;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

extern size_t						LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET;

typedef bool						(*lodge_make_property_widget_func_t)(struct nk_context *ctx, struct lodge_property *property, void *object);

void								lodge_gui_property_widget_factory_init();

lodge_make_property_widget_func_t	lodge_type_get_make_property_widget_func(lodge_type_t type);
void								lodge_type_set_make_property_widget_func(lodge_type_t type, lodge_make_property_widget_func_t func);

bool								lodge_gui_property_widget_factory_make_tree(struct nk_context *ctx, strview_t node_label, int node_id, void *object, struct lodge_properties *properties);

#endif