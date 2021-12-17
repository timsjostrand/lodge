#pragma once

#include <stdbool.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

struct lodge_script_component
{
	bool					enabled;
	lodge_asset_t			graph_asset;
};

lodge_component_type_t		lodge_script_component_type_register(lodge_type_t lodge_type_graph_asset);