#ifndef _LODGE_ASSETS_H
#define _LODGE_ASSETS_H

#if 0

#include "strview.h"
#include "lodge_asset_id.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_assets;

struct lodge_asset_handle
{
	lodge_asset_id_t		id;
	struct lodge_assets		*assets;
};

struct lodge_assets_desc
{
	strview_t				name;
	size_t					size;

	bool					(*new_inplace)(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, void *data, size_t data_size);
	bool					(*reload_inplace)(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, void *data);
	void					(*free_inplace)(struct lodge_assets *assets, strview_t name, lodge_asset_id_t id, void *data);
};

void						lodge_assets_new_inplace(struct lodge_assets *assets, struct lodge_assets_desc desc);
void						lodge_assets_free_inplace(struct lodge_assets *assets);
size_t						lodge_assets_sizeof();

struct lodge_assets*		lodge_assets_new(struct lodge_assets_desc desc);
void						lodge_assets_free(struct lodge_assets *assets);

const void*					lodge_assets_get(struct lodge_assets *assets, strview_t name);
const void*					lodge_assets_get_by_id(struct lodge_assets *assets, lodge_asset_id_t id);
void						lodge_assets_release(struct lodge_assets *assets, strview_t name);
void						lodge_assets_reload(struct lodge_assets *assets, strview_t name);

const void*					lodge_assets_get_depend(struct lodge_assets *assets, strview_t name, struct lodge_asset_handle dependency);
void						lodge_assets_release_depend(struct lodge_assets *assets, strview_t name, struct lodge_asset_handle dependency);
void						lodge_assets_clear_dependency(struct lodge_assets *assets, struct lodge_asset_handle dependency);

void						lodge_assets_set_userdata(struct lodge_assets *assets, size_t index, void *userdata);
void*						lodge_assets_get_userdata(struct lodge_assets *assets, size_t index);

strview_t					lodge_assets_id_to_name(struct lodge_assets *assets, lodge_asset_id_t id);
lodge_asset_id_t			lodge_assets_name_to_id(struct lodge_assets *assets, strview_t name);

uint32_t					lodge_assets_get_count(struct lodge_assets *assets);
strview_t					lodge_assets_index_to_name(struct lodge_assets *assets, uint32_t index);

strview_t					lodge_asset_handle_find_name(struct lodge_asset_handle *handle);

#endif

#endif
