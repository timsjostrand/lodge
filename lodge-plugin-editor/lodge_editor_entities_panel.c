#include "lodge_editor_entities_panel.h"

#include "membuf.h"
#include "gruvbox.h"

#include "lodge_gui.h"
#include "lodge_scene_editor.h"
#include "lodge_plugin_editor.h"
#include "lodge_entity_type.h"
#include "lodge_component_type.h"
#include "lodge_window.h"
#include "lodge_scene.h"
#include "lodge_editor_selection_system.h"
#include "lodge_scene_serialize.h"

static bool lodge_gui_is_shortcut_down(lodge_gui_t gui, enum nk_keys modifier, char key)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	if(nk_input_is_key_down(&ctx->input, modifier)) {
		for(int i = 0; i < ctx->input.keyboard.text_len; i++) {
			if(ctx->input.keyboard.text[i] == key) {
				return true;
			}
		}
	}

	return false;
}

void lodge_editor_entities_panel_update(struct lodge_scene_editor *panel, lodge_gui_t gui, struct lodge_editor *editor, float dt)
{
	struct lodge_scene *scene = lodge_editor_get_current_scene(editor);
	if(!scene) {
		return;
	}

	{
		if(lodge_gui_is_shortcut_down(gui, NK_KEY_CTRL, 'd')) {
			ASSERT_FAIL("hello");
		}
	}

	//
	// GUI
	//
	{
		struct nk_context *ctx = lodge_gui_to_ctx(gui);
		const size_t prev_selected_count = panel->selected_count;
		nk_layout_row_dynamic(ctx, 0, 2);

		//
		// Add entity popup
		//
		{
			if(nk_combo_begin_label(ctx, "Add entity", nk_vec2(320, 240))) {
				nk_layout_row_dynamic(ctx, 0, 1);

				for(size_t i = 0, count = lodge_entity_types_get_count(); i < count; i++) {
					lodge_entity_type_t entity_type = lodge_entity_types_get_index(i);
					strview_t entity_type_name = lodge_entity_type_get_name(entity_type);

					if(nk_combo_item_text(ctx, entity_type_name.s, entity_type_name.length, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE)) {
						lodge_entity_t added = lodge_scene_add_entity_from_type(scene, entity_type);
						if(added) {
							lodge_scene_editor_set_entity_selected(panel, added, true);
						}

						nk_combo_close(ctx);
					}
				}

				nk_combo_end(ctx);
			}
		}

		//
		// Add Component popup
		//
		{
			if(nk_combo_begin_label(ctx, "Add component", nk_vec2(320, 240))) {
				nk_layout_row_dynamic(ctx, 0, 1);

				for(size_t i = 0, count = lodge_component_types_get_count(); i < count; i++) {
					lodge_component_type_t component_type = lodge_component_types_get_index(i);
					strview_t component_type_name = lodge_component_type_get_name(component_type);

					if(nk_combo_item_text(ctx, component_type_name.s, component_type_name.length, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE)) {
						for(size_t i = 0; i < prev_selected_count; i++) {
							lodge_entity_t selected = panel->selected[i];
							lodge_scene_add_entity_component(scene, selected, component_type);
						}

						nk_combo_close(ctx);
					}
				}

				nk_combo_end(ctx);
			}
		}

		//
		// Copy entities
		//
		if(nk_button_label(ctx, "Copy")) {
			lodge_window_t window = lodge_editor_get_window(editor);
			if(window) {
				for(size_t i=0; i<prev_selected_count; i++) {
					const lodge_entity_t selected = panel->selected[i];
				
					size_t text_size = 0;
					char *text = lodge_scene_entity_to_text(scene, selected, &text_size);
					lodge_window_set_clipboard(window, strview_make(text, text_size));
					free(text);
				}
			}
		}

		if(nk_button_label(ctx, "Paste")) {
			lodge_window_t window = lodge_editor_get_window(editor);
			if(window) {
				strview_t clipboard = lodge_window_get_clipboard(window);

				if(!strview_empty(clipboard)) {
					lodge_entity_t entity = lodge_scene_entity_from_text(scene, clipboard);
					ASSERT(entity);
				}
			}
		}

		if(nk_tree_push(ctx, NK_TREE_TAB, "Entities", NK_MAXIMIZED)) {
			lodge_scene_entities_foreach(scene, entity) {
				const bool was_selected = lodge_scene_editor_is_entity_selected(panel, entity);

				if(was_selected) {
					nk_style_push_color(ctx, &ctx->style.tab.text, nk_color_from_vec4(GRUVBOX_BRIGHT_YELLOW));
				}
				
				enum nk_collapse_states collapse_state = was_selected ? NK_MAXIMIZED : NK_MINIMIZED;
				if(nk_tree_state_push(ctx, NK_TREE_NODE, lodge_scene_get_entity_name(scene, entity).s, &collapse_state)) {
					if(!was_selected) {
						lodge_scene_editor_set_entity_selected(panel, entity, true);
					}
					nk_tree_pop(ctx);
				} else {
					if(was_selected) {
						lodge_scene_editor_set_entity_selected(panel, entity, false);
					}
				}

				if(was_selected) {
					nk_style_pop_color(ctx);
				}
			}
			nk_tree_pop(ctx);
		}
	}

	//
	// Update selected component union
	//
	// TODO(TS): only update on selection change
	//
	{
		struct component_union *component_union = &panel->component_union;
		component_union->count = 0;
		if(panel->selected_count > 0) {
			lodge_scene_entity_components_foreach(scene, panel->selected[0], first_it) {
				membuf_append(membuf_wrap(component_union->elements), &component_union->count, &first_it.type, sizeof(lodge_component_type_t));
			}

			for(size_t entity_index = 1, count = panel->selected_count; entity_index < count; entity_index++) {
				for(size_t component_index = 0; component_index < component_union->count; component_index++) {
					if(!lodge_scene_get_entity_component(scene, panel->selected[entity_index], component_union->elements[component_index])) {
						membuf_delete(membuf_wrap(component_union->elements), &component_union->count, component_index, 1);
						component_index--;
					}
				}
			}
		}

		//
		// Update impostor properties
		//
		for(size_t component_index = 0, count = component_union->count; component_index < count; component_index++) {
			lodge_component_type_t type = component_union->elements[component_index];
			struct lodge_properties *properties = lodge_component_type_get_properties(type);

			struct lodge_variant *impostor_variant = &component_union->impostor_variants[component_index];

			const void *mirror_component = lodge_scene_get_entity_component(scene, panel->selected[0], type);
			lodge_variant_set_type(impostor_variant, lodge_component_type_get_type(type), mirror_component);

			component_union->impostors[component_index] = *properties;
			for(size_t property_index = 0, j_count = properties->count; property_index < j_count; property_index++) {
				struct lodge_property *impostor = &component_union->impostors[component_index].elements[property_index];
				impostor->on_modified = &lodge_scene_editor_impostor_on_modified;
				impostor->pre_modified = &lodge_scene_editor_impostor_pre_modified;
				//impostor->flags |= LODGE_PROPERTY_FLAG_WRITE_ONLY;
				impostor->userdata = panel;
			}
		}
	}
}
