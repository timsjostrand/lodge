#include "lodge_file_picker_widget.h"

#include "dynbuf.h"

#include "lodge_vfs.h"
#include "lodge_scene.h"

#include "lodge_gui.h"

bool lodge_editor_vfs_file_picker_widget(struct lodge_gui *gui, struct lodge_vfs *vfs, strview_t path, strview_t mask, struct lodge_vfs_entry *entry_out)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	struct lodge_vfs_iterate_dynbuf files = { 0 };

	lodge_vfs_iterate(vfs, path, mask, &files);

	nk_layout_row_dynamic(ctx, 0, 1);

	bool ret = false;

	for(size_t i = 0; i < files.count; i++) {
		struct lodge_vfs_entry *file = &files.elements[i];
		if(nk_button_label(ctx, file->name)) {
			*entry_out = *file;
			ret = true;
		}
	}

	dynbuf_free_inplace(dynbuf_wrap((&files)));

	return ret;
}

enum lodge_editor_popup lodge_editor_vfs_file_picker_popup(struct lodge_gui *gui, vec2 pos, struct lodge_vfs *vfs, strview_t path, strview_t mask, struct lodge_vfs_entry *entry_out)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);
	enum lodge_editor_popup status = LODGE_EDITOR_POPUP_OPEN;
	struct nk_rect popup_rect = (struct nk_rect){ .x = pos.x, .y = pos.y, .w = 320, .h = 240 };
	if(nk_popup_begin(ctx, NK_POPUP_DYNAMIC, "File Picker", NK_WINDOW_CLOSABLE, popup_rect)) {
		if(lodge_editor_vfs_file_picker_widget(gui, vfs, path, mask, entry_out)) {
			status = LODGE_EDITOR_POPUP_OK;
		}
		nk_popup_end(ctx);
	} else {
		status = LODGE_EDITOR_POPUP_CANCEL;
	}
	return status;
}
