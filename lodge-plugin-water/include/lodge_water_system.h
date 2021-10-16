#ifndef _LODGE_WATER_SYSTEM_H
#define _LODGE_WATER_SYSTEM_H

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_type_t;
typedef struct lodge_type* lodge_type_t;

struct lodge_plugin_water;

lodge_system_type_t		lodge_water_system_type_register(struct lodge_plugin_water *plugin, lodge_type_t shader_asset_type);

#endif