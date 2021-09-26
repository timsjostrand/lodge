#ifndef _LODGE_TERRAIN_COMPONENT_H
#define _LODGE_TERRAIN_COMPONENT_H

#include "math4.h"

#include "lodge_texture.h"
#include "lodge_tesselated_plane.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_static_material
{
	lodge_asset_t					albedo;
	lodge_asset_t					normal;
	lodge_asset_t					displacement;
	lodge_asset_t					metalness;
};

struct lodge_terrain_chunk
{
	vec2							offset;
	vec3							world_offset;
	vec3							world_size;
	bool							visible;
	float							lod;
};

struct lodge_terrain_component
{
	uint32_t						chunks_x;
	uint32_t						chunks_y;
	vec3							chunk_size;

	lodge_asset_t					heightmap;
	struct lodge_static_material	material;

	struct lodge_terrain_chunk		*chunks;

	struct lodge_tesselated_plane	*plane;
};

extern lodge_component_type_t		LODGE_COMPONENT_TYPE_TERRAIN;

lodge_component_type_t				lodge_terrain_component_type_register(lodge_type_t texture_asset_type);

inline struct lodge_terrain_chunk*	lodge_terrain_component_get_chunk(struct lodge_terrain_component *terrain, uint32_t x, uint32_t y)
{
	return &terrain->chunks[y * terrain->chunks_y + x];
}

#endif