#ifndef _LODGE_EDITOR_SELECTION_SYSTEM_H
#define _LODGE_EDITOR_SELECTION_SYSTEM_H

#include <stdbool.h>

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_editor_selection_system;

lodge_system_type_t	lodge_editor_selection_system_type_register(struct lodge_plugin_editor *plugin);

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

void				lodge_scene_set_entity_selected(lodge_scene_t scene, lodge_entity_t entity, bool selected);
bool				lodge_scene_is_entity_selected(lodge_scene_t scene, lodge_entity_t entity);
lodge_entity_t*		lodge_scene_selected_it_begin(lodge_scene_t scene);
lodge_entity_t*		lodge_scene_selected_it_next(lodge_scene_t scene, lodge_entity_t *it);

#endif