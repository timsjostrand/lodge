#ifndef _LODGE_EDITOR_PLUGINS_PANEL_H
#define _LODGE_EDITOR_PLUGINS_PANEL_H

#include <stddef.h>

struct lodge_plugins_panel;

struct lodge_editor;

struct lodge_plugins;

struct lodge_gui;
typedef struct lodge_gui* lodge_gui_t;

void	lodge_plugins_panel_new_inplace(struct lodge_plugins_panel *panel, struct lodge_plugins *plugins);
void	lodge_plugins_panel_free_inplace(struct lodge_plugins_panel *panel);
size_t	lodge_plugins_panel_sizeof();

void	lodge_plugins_panel_update(struct lodge_plugins_panel *panel, lodge_gui_t gui, struct lodge_editor *plugin, float dt);
void	lodge_stats_panel_update(struct lodge_plugins_panel *panel, lodge_gui_t gui, struct lodge_editor *editor, float dt);

#endif