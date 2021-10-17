#include "lodge_editor_systems_panel.h"

#include "lodge_scene_editor.h"
#include "lodge_gui.h"
#include "lodge_gui_property_widget_factory.h"
#include "lodge_plugin_editor.h"
#include "lodge_system_type.h"
#include "lodge_scene.h"

void lodge_editor_systems_panel_update(struct lodge_scene_editor *panel, struct lodge_gui *gui, struct lodge_editor *editor, float dt)
{
	struct lodge_scene *scene = lodge_editor_get_current_scene(editor);
	if(!scene) {
		return;
	}

	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	//if(nk_begin(ctx, "Systems", nk_rect(1920-500, 540, 500, 540), NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)) {
	nk_layout_row_dynamic(ctx, 0, 1);

	if(nk_combo_begin_label(ctx, "Add system...", nk_vec2(320, 240))) {
		nk_layout_row_dynamic(ctx, 0, 1);
		for(lodge_system_type_t it = lodge_system_type_it_begin(); it; it = lodge_system_type_it_next(it)) {
			strview_t system_name = lodge_system_type_get_name(it);
			if(nk_combo_item_text(ctx, system_name.s, system_name.length, NK_TEXT_ALIGN_LEFT)) {
				lodge_scene_add_system(scene, it);
			}
		}
		nk_combo_end(ctx);
	}
		
	lodge_scene_systems_foreach(scene, it) {
		strview_t type_name = lodge_system_type_get_name(it.type);
		// FIXME(TS): bogus ids
		const int system_id = (int)it.type;
		lodge_gui_property_widget_factory_make_tree(ctx, type_name, system_id, it.value, lodge_system_type_get_properties(it.type));
	}
	//}
	//nk_end(ctx);
}

