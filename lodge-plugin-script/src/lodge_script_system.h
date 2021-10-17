#ifndef _LODGE_SCRIPT_SYSTEM_H
#define _LODGE_SCRIPT_SYSTEM_H

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_plugin_script;

lodge_system_type_t lodge_script_system_type_register(struct lodge_plugin_script *plugin);

#endif