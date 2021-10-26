#ifndef _LODGE_TYPE_ASSET
#define _LODGE_TYPE_ASSET

#include "strview.h"

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

struct lodge_type_asset_desc;
struct lodge_assets2;

typedef void (*lodge_type_asset_edit_func_t)(struct lodge_assets2 *assets, lodge_asset_t asset);

//
// FIXME(TS): consistent naming...
// 
// LODGE_TYPE_ASSET_TYPE vs LODGE_TYPE_ASSET vs LODGE_ASSET_TYPE
//

extern lodge_type_t				LODGE_TYPE_ASSET_DESC;

lodge_type_t					lodge_type_register_asset(strview_t name, struct lodge_assets2 *assets);
const void*						lodge_type_get_asset(lodge_type_t type, lodge_asset_t asset);
void*							lodge_type_access_asset(lodge_type_t type, lodge_asset_t asset);
struct lodge_assets2*			lodge_type_get_assets(lodge_type_t type);

struct lodge_assets2*			lodge_type_asset_desc_get_assets(struct lodge_type_asset_desc *desc);

lodge_type_asset_edit_func_t	lodge_type_asset_get_edit_func(lodge_type_t asset_type);
void							lodge_type_asset_set_edit_func(lodge_type_t asset_type, lodge_type_asset_edit_func_t edit_func);

#endif