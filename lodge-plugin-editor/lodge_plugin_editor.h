#ifndef _LODGE_PLUGIN_EDITOR_H
#define _LODGE_PLUGIN_EDITOR_H

#include "lodge_plugin.h"

#include "strview.h"

struct lodge_editor;

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_asset;
typedef struct lodge_asset* lodge_asset_t;

struct lodge_window;
typedef struct lodge_window* lodge_window_t;

typedef struct lodge_gui* lodge_gui_t;

struct lodge_scene;

typedef void					(*lodge_editor_panel_func_t)(void *userdata, lodge_gui_t gui, struct lodge_editor *editor, float dt);

struct lodge_editor_types
{
	lodge_system_type_t			selection_system_type;

	lodge_system_type_t			controller_system_type;
	lodge_component_type_t		controller_component_type;
};

struct lodge_editor_panel_desc
{
	strview_t					name;
	void						*panel;
	vec2						pos;
	vec2						size;
	lodge_editor_panel_func_t	update;
	bool						allow_instances;
	bool						hide_scrollbar;
};

struct lodge_plugin_desc		lodge_plugin_editor();

struct lodge_editor_types		lodge_editor_get_types(struct lodge_editor *editor);

void							lodge_editor_set_enabled(struct lodge_editor *editor, bool enabled);

void							lodge_editor_add_panel(struct lodge_editor *editor, struct lodge_editor_panel_desc *panel_desc);
void							lodge_editor_remove_panel(struct lodge_editor *editor, strview_t name);

struct lodge_scene*				lodge_editor_get_current_scene(struct lodge_editor *editor);
void							lodge_editor_set_current_scene(struct lodge_editor *editor, lodge_asset_t scene_asset);
lodge_asset_t					lodge_editor_get_current_scene_asset(struct lodge_editor *editor);
struct lodge_assets2*			lodge_editor_get_scenes(struct lodge_editor *editor);

lodge_window_t					lodge_editor_get_window(struct lodge_editor *editor);

void							lodge_editor_set_controller(struct lodge_editor *editor, struct lodge_editor_controller_component *controller);

#endif
