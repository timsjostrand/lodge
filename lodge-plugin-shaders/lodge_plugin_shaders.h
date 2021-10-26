#ifndef _LODGE_PLUGIN_SHADERS_H
#define _LODGE_PLUGIN_SHADERS_H

#include "lodge_plugin.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_assets2;

struct shader_types
{
	lodge_type_t			shader_asset_type;
};

struct shader_types			lodge_plugin_shaders_get_types(struct lodge_assets2 *shaders);

LODGE_PLUGIN_DECL(lodge_plugin_shaders);

#endif
