#ifndef _LODGE_FILE_PICKER_WIDGET_H
#define _LODGE_FILE_PICKER_WIDGET_H

#include "math4.h"
#include "strview.h"

#include <stdbool.h>

struct lodge_gui;
struct lodge_vfs;
struct lodge_vfs_entry;

#if 0
enum lodge_editor_popup
{
	LODGE_EDITOR_POPUP_OPEN,
	LODGE_EDITOR_POPUP_OK,
	LODGE_EDITOR_POPUP_CANCEL,
};
#endif

bool						lodge_editor_vfs_file_picker_widget(struct lodge_gui *gui, struct lodge_vfs *vfs, strview_t path, strview_t mask, struct lodge_vfs_entry *entry_out);
//enum lodge_editor_popup		lodge_editor_vfs_file_picker_popup(struct lodge_gui *gui, vec2 pos, struct lodge_vfs *vfs, strview_t path, strview_t mask, struct lodge_vfs_entry *entry_out);

#endif