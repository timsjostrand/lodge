#include "lodge_editor_scenes_panel.h"

#include "lodge_scene_editor.h"
#include "lodge_gui.h"
#include "lodge_plugin_editor.h"
#include "lodge_plugin_scenes.h"

#include "lodge_properties.h"
#include "lodge_gui_property_widgets.h"
#include "lodge_scene_serialize.h"

void lodge_editor_scenes_panel_update(struct lodge_scene_editor *panel, struct lodge_gui *gui, struct lodge_editor *editor, float dt)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	struct lodge_assets2 *scenes = lodge_editor_get_scenes(editor);
	struct lodge_scenes_types types = lodge_plugin_scenes_get_types(scenes);

	//if(nk_begin(ctx, "Scenes", nk_rect(0, 0, 300, 100), NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)) {
	nk_layout_row_dynamic(ctx, 0, 2);

	//
	// Load button
	//
	{
		lodge_asset_t new_scene = lodge_editor_get_current_scene_asset(editor);
		struct lodge_property scene_property = {
			.name = strview("scene"),
			.type = types.scene_asset_type,
		};

		lodge_make_property_widget_func_t widget_func = lodge_type_get_make_property_widget_func(types.scene_asset_type);
		if(widget_func(ctx, &scene_property, &new_scene)) {
			lodge_editor_set_current_scene(editor, new_scene);
		}
	}

	//
	// Save button
	// 
	// TODO(TS): Draw disabled button if no scene loaded
	//
	{
		if(nk_button_label(ctx, "Save")) {
			lodge_scene_t scene = lodge_editor_get_current_scene(editor);
			if(scene) {
				size_t scene_text_len = 0;
				char* scene_text = lodge_scene_to_text(scene, &scene_text_len);
				printf("%s\n", scene_text);
				free(scene_text);
			}
		}
	}
		//nk_end(ctx);
	//}
}

