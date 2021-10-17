#ifndef _LODGE_EDITOR_VIEWPORT_PANEL_H
#define _LODGE_EDITOR_VIEWPORT_PANEL_H

#include <stddef.h>

struct lodge_viewport_panel;
struct lodge_gui;
struct lodge_editor;
struct lodge_plugins;

void	lodge_editor_viewport_panel_new_inplace(struct lodge_viewport_panel *panel, struct lodge_plugins *plugins);
size_t	lodge_editor_viewport_panel_sizeof();
void	lodge_editor_viewport_panel_update(struct lodge_viewport_panel *panel, struct lodge_gui *gui, struct lodge_editor *editor, float dt);

#endif