#ifndef _LODGE_ASSETS2_H
#define _LODGE_ASSETS2_H

#include "strview.h"

#include <stdint.h>

struct lodge_assets2;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

typedef void				(*lodge_assets2_on_modified_t)(struct lodge_assets2 *assets, lodge_asset_t asset, void *userdata);

struct lodge_assets2_desc
{
	strview_t				name;
	size_t					size;

	bool					(*new_inplace)(struct lodge_assets2 *assets, strview_t name, lodge_asset_t asset, void *data);
	bool					(*new_default_inplace)(struct lodge_assets2 *assets, void *data);
	bool					(*reload_inplace)(struct lodge_assets2 *assets, strview_t name, lodge_asset_t asset, void *data);
	void					(*free_inplace)(struct lodge_assets2 *assets, strview_t name, lodge_asset_t asset, void *data);
};

void						lodge_assets2_new_inplace(struct lodge_assets2 *assets, struct lodge_assets2_desc *desc);
void						lodge_assets2_free_inplace(struct lodge_assets2 *assets);
size_t						lodge_assets2_sizeof();

lodge_asset_t				lodge_assets2_register(struct lodge_assets2 *assets, strview_t name);
lodge_asset_t				lodge_assets2_find_by_name(struct lodge_assets2 *assets, strview_t name);
lodge_asset_t				lodge_assets2_find_by_name_hash(struct lodge_assets2 *assets, uint32_t name_hash);

void						lodge_assets2_add_listener(struct lodge_assets2 *assets, lodge_asset_t asset, struct lodge_assets2 *listener_assets, lodge_asset_t listener_asset);
void						lodge_assets2_remove_listener(struct lodge_assets2 *assets, lodge_asset_t asset, struct lodge_assets2 *listener_assets, lodge_asset_t listener_asset);
void						lodge_assets2_remove_listener_by_name(struct lodge_assets2 *assets, strview_t name, struct lodge_assets2 *listener_assets, lodge_asset_t listener_asset);

//
// TODO(TS): refactor `lodge_assets2_get()` into `_async` and `_block` variants. _async will return a
// Asset Type Default object while waiting for the asset to load, and _block will wait until the asset
// is loaded.
//

void*						lodge_assets2_get(struct lodge_assets2 *assets, lodge_asset_t asset);
strview_t					lodge_assets2_get_name(struct lodge_assets2 *assets, const lodge_asset_t asset);

lodge_asset_t				lodge_assets2_make_default(struct lodge_assets2 *assets);
void						lodge_assets2_set(struct lodge_assets2 *assets, lodge_asset_t asset, const void *src);
void						lodge_assets2_invalidate(struct lodge_assets2 *assets, lodge_asset_t asset);

void						lodge_assets2_set_userdata(struct lodge_assets2 *assets, int userdata_index, void *userdata);
void*						lodge_assets2_get_userdata(struct lodge_assets2 *assets, int userdata_index);

lodge_asset_t				lodge_assets2_it_begin(struct lodge_assets2 *assets);
lodge_asset_t				lodge_assets2_it_next(struct lodge_assets2 *assets, lodge_asset_t it);

#endif