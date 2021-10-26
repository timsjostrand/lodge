#include "lodge_asset_id.h"

#if 0

#include "lodge_platform.h"

lodge_asset_id_t lodge_asset_id_make_invalid()
{
	return 0;
}

lodge_asset_id_t lodge_asset_id_make(uint32_t hash)
{
	return (uint64_t)LODGE_BIT(64) | (uint64_t)hash;
}

bool lodge_asset_id_is_valid(lodge_asset_id_t id)
{
	// Get 1st bit
	return id >> 63;
}

uint32_t lodge_asset_id_get_hash(lodge_asset_id_t id)
{
	// Get lower 32 bits
	return (uint32_t)id;
}

uint32_t lodge_asset_id_get_reserved_uint31(lodge_asset_id_t id)
{
	// Remove hash + valid bit
	return (id >> 32) & 0x7FFFFFFF;
}

#endif