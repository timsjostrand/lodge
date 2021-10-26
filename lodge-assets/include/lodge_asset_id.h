#ifndef _LODGE_ASSET_ID_H
#define _LODGE_ASSET_ID_H

#if 0

#include <stdint.h>
#include <stdbool.h>

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

lodge_asset_id_t			lodge_asset_id_make_invalid();
lodge_asset_id_t			lodge_asset_id_make(uint32_t hash);
bool						lodge_asset_id_is_valid(lodge_asset_id_t id);
uint32_t					lodge_asset_id_get_hash(lodge_asset_id_t id);
uint32_t					lodge_asset_id_get_reserved_uint31(lodge_asset_id_t id);

#endif

#endif