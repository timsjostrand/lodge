#ifndef _LODGE_PLUGIN_TEXTURES_H
#define _LODGE_PLUGIN_TEXTURES_H

#include "lodge_plugin.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_assets2;

struct texture_types
{
	lodge_type_t			texture_asset_type;
};

struct texture_types		lodge_plugin_textures_get_types(struct lodge_assets2 *textures);

LODGE_PLUGIN_DECL(lodge_plugin_textures);

#endif