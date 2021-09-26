#ifndef _LODGE_PLUGIN_FBX_H
#define _LODGE_PLUGIN_FBX_H

#include "lodge_plugin.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct fbx_types
{
	lodge_type_t fbx_asset_type;
};

struct lodge_plugin_desc	lodge_plugin_fbx();
struct fbx_types			lodge_plugin_fbx_get_types(struct lodge_assets2 *fbx_assets);

#endif