#ifndef _LODGE_BILLBOARD_SYSTEM_H
#define _LODGE_BILLBOARD_SYSTEM_H

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_scene_renderer_plugin;

lodge_system_type_t lodge_billboard_system_type_register(struct lodge_scene_renderer_plugin *plugin);

#endif