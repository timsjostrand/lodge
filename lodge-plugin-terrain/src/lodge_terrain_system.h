#ifndef _LODGE_TERRAIN_SYSTEM_H
#define _LODGE_TERRAIN_SYSTEM_H

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_plugin_terrain;

enum lodge_terrain_lod_level
{
	LODGE_TERRAIN_LOD_LEVEL_128 = 0,
	LODGE_TERRAIN_LOD_LEVEL_64,
	LODGE_TERRAIN_LOD_LEVEL_32,
	LODGE_TERRAIN_LOD_LEVEL_16,
	LODGE_TERRAIN_LOD_LEVEL_8,
	LODGE_TERRAIN_LOD_LEVEL_4,
	LODGE_TERRAIN_LOD_LEVEL_2,
	LODGE_TERRAIN_LOD_LEVEL_1,
	LODGE_TERRAIN_LOD_LEVEL_MAX,
};

lodge_system_type_t lodge_terrain_system_type_register(struct lodge_plugin_terrain *plugin);

#endif