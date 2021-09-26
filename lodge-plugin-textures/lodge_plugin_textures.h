#ifndef _LODGE_PLUGIN_TEXTURES_H
#define _LODGE_PLUGIN_TEXTURES_H

#include "lodge_plugin.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct texture_types
{
	lodge_type_t			texture_asset_type;
};

struct lodge_plugin_desc	lodge_plugin_textures();
struct texture_types		lodge_plugin_textures_get_types(struct lodge_assets2 *textures);

#endif