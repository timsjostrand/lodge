#include "lodge_editor_properties_panel.h"

#include "lodge_scene_editor.h"
#include "lodge_gui.h"
#include "lodge_gui_property_widget_factory.h"
#include "lodge_plugin_editor.h"
#include "lodge_scene.h"

#include "lodge_component_type.h"

static int lodge_component_type_to_gui_id(lodge_component_type_t component_type)
{
	return (int)((intptr_t)component_type);
}

void lodge_editor_properties_panel_update(struct lodge_scene_editor *panel, lodge_gui_t gui, struct lodge_editor *editor, float dt)
{
	lodge_scene_t scene = lodge_editor_get_current_scene(editor);
	if(!scene) {
		return;
	}

	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	//if(nk_begin(ctx, "Properties", nk_rect(1920-500, 0, 500, 540), NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)) {
#if 1
	for(size_t i = 0; i < panel->component_union.count; i++) {
		//if(nk_tree_push(ctx, NK_TREE_TAB, lodge_component_type_get_name(editor->component_union.elements[i]).s, NK_MAXIMIZED)) {
		//	nk_tree_pop(ctx);
		//}

		lodge_component_type_t component_type = panel->component_union.elements[i];

		strview_t type_name = lodge_component_type_get_name(component_type);
		// FIXME(TS): bogus ids
		const int component_id = lodge_component_type_to_gui_id(component_type);
		void* component = lodge_scene_get_entity_component(scene, panel->selected[0], component_type);

		lodge_gui_property_widget_factory_make_tree(ctx, type_name, component_id, component, &panel->component_union.impostors[i]);
	}
#else
	for(size_t i=0, count=editor->selected_count; i < count; i++) {
		lodge_entity_t entity = editor->selected[i];
		lodge_scene_entity_components_foreach(scene, entity, component_it) {
			strview_t type_name = lodge_component_type_get_name(component_it.type);
			// FIXME(TS): bogus ids
			const int component_id = (int)entity + component_it.index*1000;
			make_properties_tree(ctx, type_name, component_id, component_it.value, lodge_component_type_get_properties(component_it.type));
		}
	}
#endif
	//}
	//nk_end(ctx);
}
