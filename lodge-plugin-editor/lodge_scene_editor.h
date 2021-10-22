#ifndef _LODGE_SCENE_EDITOR_H
#define _LODGE_SCENE_EDITOR_H

#include "lodge_properties.h"
#include "lodge_variant.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_scene_editor;

struct lodge_gui;
typedef struct lodge_gui* lodge_gui_t;

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_window;
typedef struct lodge_window* lodge_window_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_scene_editor;

//
// FIXME(TS): should private these structs and implement proper API -- but `scene_editor.h` is private already so no rush.
//

struct component_union
{
	size_t								count;
	lodge_component_type_t				elements[128];

	struct lodge_properties				impostors[128];
	struct lodge_variant				impostor_variants[128];
};

struct lodge_scene_editor
{
	//lodge_scene_t						scene;
	struct lodge_editor					*editor;

	lodge_entity_t						selected[1024];
	size_t								selected_count;

	struct component_union				component_union;
};

struct lodge_component_type_property
{
	lodge_component_type_t				type;
	struct lodge_property				*property;
};

void									lodge_scene_editor_new_inplace(struct lodge_scene_editor *editor, struct lodge_editor *owner);
void									lodge_scene_editor_free_inplace(struct lodge_scene_editor *editor);
size_t									lodge_scene_editor_sizeof();

struct lodge_component_type_property	lodge_scene_editor_property_find_component_type(struct lodge_scene_editor *editor, struct lodge_property *property);

void									lodge_scene_editor_impostor_on_modified(struct lodge_property *property, const void *object);
bool									lodge_scene_editor_impostor_pre_modified(struct lodge_property *property, const void *object, const void *dst, const void *src);

bool									lodge_scene_editor_is_entity_selected(struct lodge_scene_editor *editor, lodge_entity_t entity);
void									lodge_scene_editor_set_entity_selected(struct lodge_scene_editor *editor, lodge_entity_t entity, bool selected);

//void	lodge_scene_editor_update(struct lodge_scene_editor *panel, lodge_gui_t gui, struct lodge_editor *editor, float dt);

#endif
