#ifndef _LODGE_ASSETS_H
#define _LODGE_ASSETS_H

#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_assets;

// Asset ID type.
//
// Designed so that the literal `0` will always be an invalid Asset ID, and
// if two IDs compare equal they will be referring to the same resource.
//
// Bit layout:
//
//		[64..64]		Valid flag: true/false.
//		[63..31]		Reserved for future implementation of unique counter.
//		[32..00]		Name hash.
//
typedef uint64_t lodge_asset_id_t;

#define LODGE_ASSET_ID_FMT "{ %u (valid: %d, hash: %u) }"

#define LODGE_ASSET_ID_ARG(asset_id) lodge_asset_id_is_valid(asset_id), lodge_asset_id_get_hash(asset_id)

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
void						lodge_assets_release(struct lodge_assets *assets, strview_t name);
void						lodge_assets_reload(struct lodge_assets *assets, strview_t name);

const void*					lodge_assets_get_depend(struct lodge_assets *assets, strview_t name, struct lodge_asset_handle dependency);
void						lodge_assets_release_depend(struct lodge_assets *assets, strview_t name, struct lodge_asset_handle dependency);
void						lodge_assets_clear_dependency(struct lodge_assets *assets, struct lodge_asset_handle dependency);

void						lodge_assets_set_userdata(struct lodge_assets *assets, size_t index, void *userdata);
void*						lodge_assets_get_userdata(struct lodge_assets *assets, size_t index);

strview_t					lodge_assets_id_to_name(struct lodge_assets *assets, lodge_asset_id_t id);
lodge_asset_id_t			lodge_assets_name_to_id(struct lodge_assets *assets, strview_t name);

strview_t					lodge_asset_handle_find_name(struct lodge_asset_handle *handle);

lodge_asset_id_t			lodge_asset_id_make_invalid();
lodge_asset_id_t			lodge_asset_id_make(uint32_t hash);
bool						lodge_asset_id_is_valid(lodge_asset_id_t id);
uint32_t					lodge_asset_id_get_hash(lodge_asset_id_t id);
uint32_t					lodge_asset_id_get_reserved_uint31(lodge_asset_id_t id);

#endif
