#pragma once

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_plugin_script;

lodge_system_type_t lodge_script_system_type_register(struct lodge_plugin_script *plugin);