#include "lodge_type_asset.h"

#include "str.h"
#include "strview.h"

#include "lodge_type.h"
#include "lodge_platform.h"
#include "lodge_assets2.h"

lodge_type_t LODGE_TYPE_ASSET_DESC = NULL;

struct lodge_type_asset_desc
{
	struct lodge_assets2 *assets;
};

lodge_type_t lodge_type_register_asset(strview_t name, struct lodge_assets2 *assets)
{
	if(!LODGE_TYPE_ASSET_DESC) {
		LODGE_TYPE_ASSET_DESC = lodge_type_register(strview_static("lodge_asset_ref_desc"), sizeof(struct lodge_type_asset_desc));
	}
	return lodge_type_register_with_userdata(
		name,
		sizeof(lodge_asset_t),
		LODGE_TYPE_ASSET_DESC,
		&(struct lodge_type_asset_desc) {
			.assets = assets,	
		}
	);
}

struct lodge_assets2* lodge_type_get_assets(lodge_type_t type)
{
	ASSERT_OR(type) { return NULL; }
	ASSERT_OR(LODGE_TYPE_ASSET_DESC) { return NULL; }
	struct lodge_type_asset_desc *desc = lodge_type_get_userdata(type, LODGE_TYPE_ASSET_DESC);
	ASSERT_OR(desc) { return NULL; }
	return desc->assets;
}

// TODO(TS): asset_ref->ptr = NULL on name modify
// -- also keep ->ptr_valid bool?

const void* lodge_type_get_asset(lodge_type_t type, lodge_asset_t asset)
{
	ASSERT_OR(type) { return NULL; }
	ASSERT_OR(LODGE_TYPE_ASSET_DESC) { return NULL; }
	if(!asset) { return NULL; }

	struct lodge_type_asset_desc *desc = lodge_type_get_userdata(type, LODGE_TYPE_ASSET_DESC);
	ASSERT_OR(desc) { return NULL; }
	return lodge_assets2_get(desc->assets, asset);
}

void* lodge_type_access_asset(lodge_type_t type, lodge_asset_t asset)
{
	return (void*)lodge_type_get_asset(type, asset);
}

struct lodge_assets2* lodge_type_asset_desc_get_assets(struct lodge_type_asset_desc *desc)
{
	return desc ? desc->assets : NULL;
}

size_t LODGE_TYPE_FUNC_INDEX_EDIT_ASSET = 0;

lodge_type_asset_edit_func_t lodge_type_asset_get_edit_func(lodge_type_t asset_type)
{
	ASSERT_OR(asset_type) { return NULL; }
	if(!LODGE_TYPE_FUNC_INDEX_EDIT_ASSET) {
		LODGE_TYPE_FUNC_INDEX_EDIT_ASSET = lodge_types_make_func_index();
	}
	return lodge_type_get_func(asset_type, LODGE_TYPE_FUNC_INDEX_EDIT_ASSET);
}

void lodge_type_asset_set_edit_func(lodge_type_t asset_type, lodge_type_asset_edit_func_t edit_func)
{
	if(!LODGE_TYPE_FUNC_INDEX_EDIT_ASSET) {
		LODGE_TYPE_FUNC_INDEX_EDIT_ASSET = lodge_types_make_func_index();
	}
	lodge_type_set_func(asset_type, LODGE_TYPE_FUNC_INDEX_EDIT_ASSET, edit_func);
}
