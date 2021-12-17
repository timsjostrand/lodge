#include "lodge_ns_editor.h"

#include "lodge_type.h"
#include "lodge_variant.h"
#include "lodge_ns_graph.h"
#include "lodge_ns_node.h"
#include "lodge_ns_node_type.h"
#include "lodge_gui_property_widgets.h"
#include "lodge_properties.h"

#include "lodge_json.h"
#include "lodge_serialize_json.h"
#include "lodge_ns_graph_serialize.h"

#include "lodge_gui.h"

#include "lodge_platform.h"

#include <stdint.h>
#include <float.h>
#include <string.h>

struct lodge_ns_editor_pin
{
	vec2						pos;
	lodge_type_t				type;
};

struct lodge_ns_editor_pins
{
	struct lodge_ns_editor_pin	inputs[8];
	struct lodge_ns_editor_pin	outputs[8];
};

struct lodge_ns_editor
{
	lodge_graph_t					graph;
	bool							modified;

	struct lodge_pin_connection		editing_link;

	char							add_node_filter[255];
	int								add_node_filter_len;

	struct nk_rect					node_bounds[256];

	struct lodge_ns_editor_pins		node_pins[256];

	void							*userdata;
	lodge_ns_editor_on_save_func_t	on_save;
};

static struct nk_color node_editor_pin_type_to_color(lodge_type_t type)
{
	if(type == LODGE_TYPE_BOOL) {
		return nk_rgba_hex("#fabd2fff");
	} else if(type == LODGE_TYPE_F32) {
		return nk_rgba_hex("#8ec07cff");
	} else if(type == LODGE_TYPE_VEC3) {
		return nk_rgba_hex("#83a598ff");
	}

	return nk_rgba_hex("#ffffffff");
}

static void node_editor_update(struct lodge_ns_editor *editor, lodge_gui_t gui, float dt)
{
	ASSERT_OR(editor && gui) { return; }

	struct nk_context *ctx = lodge_gui_to_ctx(gui);
	lodge_graph_t graph = editor->graph;
	void* graph_context = lodge_graph_get_context(graph);

    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    struct nk_rect total_space = nk_window_get_content_region(ctx);
	const size_t nodes_count = lodge_graph_get_node_count(graph);

	if(editor->on_save) {
		nk_layout_row_dynamic(ctx, 30, 1);
		if(nk_button_label(ctx, "Save")) {
			//size_t text_size;
			//char* text = lodge_graph_to_text(graph, &text_size);
	
			//lodge_graph_t new_graph = lodge_graph_from_text(strview_make(text, text_size - 1), graph_context);
			//ASSERT(new_graph);
			//LODGE_UNUSED(new_graph);
	
			//if(text) {
			//	free(text);
			//}
	
			if(editor->on_save(editor, graph, editor->userdata)) {
				editor->modified = false;
			}
		}
	}

	nk_layout_space_begin(ctx, NK_STATIC, total_space.h, nodes_count);

	//
	// Node windows
	//
	for(size_t node_idx = 0; node_idx < nodes_count; node_idx++) {
		lodge_node_t node = lodge_graph_get_node_from_index(graph, node_idx);
		lodge_node_id_t node_id = lodge_graph_get_node_id(graph, node);

		const strview_t node_type_name = lodge_node_type_get_name(node->type);
		const lodge_pin_idx_t pin_count_max = max(node->inputs.count, node->outputs.count);

		struct nk_rect *node_bounds = &editor->node_bounds[node_id];
		struct lodge_ns_editor_pins *node_pins = &editor->node_pins[node_id];

		if(node_bounds->x < 0.0f) {
			node_bounds->x = 8 + node_idx * 8;
		}
		if(node_bounds->y < 0.0f) {
			node_bounds->y = 8 + node_idx * 8;
		}
		if(node_bounds->w < 0.0f) {
			node_bounds->w = 200;
		}
		if(node_bounds->h < 0.0f) {
			node_bounds->h = 30 * (1 + pin_count_max) + 30;
		}

		nk_layout_space_push(ctx, *node_bounds);

		struct nk_panel *node_panel;

		char node_gui_id[2 + sizeof(size_t) + 1] = { 0 };
		node_gui_id[0] = 'n';
		node_gui_id[1] = '_';
		memcpy(node_gui_id+2, (const char*)&node_id, sizeof(size_t));

		//
		// Node window
		//
		if(nk_group_begin_titled(ctx, node_gui_id, node_type_name.s,
			NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
			node_panel = nk_window_get_panel(ctx);

			for(lodge_pin_idx_t pin_idx = 0; pin_idx < pin_count_max; pin_idx++) {
				nk_layout_row_dynamic(ctx, 30, 2);

				//
				// Inputs
				//
				if(pin_idx == 0 && node->inputs.count == 0 && lodge_variant_is_set(&node->config)) {
					lodge_make_property_widget_func_t make_widget_func = lodge_type_get_make_property_widget_func(node->config.type);

					if(make_widget_func) {
						struct lodge_property property = {
							.offset = 0,
							.type = node->config.type
						};

						if(make_widget_func(ctx, &property, lodge_variant_access_data_ptr(&node->config))) {
							lodge_graph_unconfigure(graph);
							//
							// FIXME(TS): we may need to clone to value to stack and then call:
							//
							//		lodge_variant_set_type(node->config, node->config.type, tmp.value);
							// 
							// instead of applying to node->config directly...
							//
							lodge_graph_configure(graph);
						}
					} else {
						nk_spacing(ctx, 1);
					}
				} else {
					if(pin_idx < node->inputs.count) {
						struct lodge_input_pin *pin = &node->inputs.pins[pin_idx];

						struct nk_rect button_bounds = nk_widget_bounds(ctx);
						node_pins->inputs[pin_idx] = (struct lodge_ns_editor_pin) {
							.pos.x = button_bounds.x,
							.pos.y = button_bounds.y + button_bounds.h / 2.0f,
							.type = pin->config.type
						};

						struct nk_style_button style_not_connected = ctx->style.button;
						style_not_connected.border_color = nk_rgba(255,0,0,255);

						const bool is_connected = lodge_node_input_is_connected(node, pin_idx);
						struct nk_style_button *style = is_connected ? &ctx->style.button : &style_not_connected;

						if(nk_button_text_styled(ctx, style, pin->config.name.s, pin->config.name.length)) {
							if(editor->editing_link.node) {
								if(lodge_node_can_connect(editor->editing_link.node, editor->editing_link.pin_index, node, pin_idx)) {
									lodge_graph_unconfigure(graph);

									lodge_node_connect(
										editor->editing_link.node, editor->editing_link.pin_index,
										node, pin_idx
									);

									lodge_graph_configure(graph);
								}

								editor->editing_link.node = NULL;
							};
						} else if(nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_RIGHT, button_bounds, nk_true)) {
							lodge_graph_unconfigure(graph);
							{
								lodge_node_input_disconnect(node, pin_idx);
							}
							lodge_graph_configure(graph);
						}
					} else {
						nk_spacing(ctx, 1);
					}
				}

				//
				// Outputs
				//
				if(pin_idx < node->outputs.count) {
					struct lodge_output_pin *pin = &node->outputs.pins[pin_idx];

					struct nk_rect button_bounds = nk_widget_bounds(ctx);
					editor->node_pins[node_id].outputs[pin_idx] = (struct lodge_ns_editor_pin) {
						.pos.x = button_bounds.x + button_bounds.w,
						.pos.y = button_bounds.y + button_bounds.h / 2.0f,
						.type = pin->config.type
					};
						
					struct nk_style_button style_reconnecting = ctx->style.button;
					style_reconnecting.border_color = nk_rgba(0,255,0,255);

					const bool is_reconnecting = editor->editing_link.node == node;
					struct nk_style_button *style = !is_reconnecting ? &ctx->style.button : &style_reconnecting;

					if(nk_button_text_styled(ctx, style, pin->config.name.s, pin->config.name.length)) {
						editor->editing_link = (struct lodge_pin_connection) {
							.node = node,
							.pin_index = pin_idx,
						};
					}
				} else {
					nk_spacing(ctx, 1);
				}
			}

			nk_group_end(ctx);

			//if(nk_window_is_active(ctx, node_gui_id)) {
				*node_bounds = nk_layout_space_rect_to_local(ctx, node_panel->bounds);
			//}
		} else {
			lodge_graph_unconfigure(graph);
			lodge_graph_remove_node(graph, node_id);
			lodge_graph_configure(graph);
			break; // stop iterating nodes
		}
	}

	const struct nk_vec2 mouse_pos_editor_space = nk_layout_space_to_local(ctx, ctx->input.mouse.pos);
	nk_layout_space_end(ctx);

	//
	// Context menu
	//
	if(nk_contextual_begin(ctx, 0, nk_vec2(240, 320), nk_window_get_bounds(ctx))) {
		nk_layout_row_dynamic(ctx, 25, 1);

		nk_edit_string(ctx, NK_EDIT_SIMPLE, &editor->add_node_filter[0], &editor->add_node_filter_len, LODGE_ARRAYSIZE(editor->add_node_filter), NULL);

		strview_t add_node_filter = strview_make(editor->add_node_filter, editor->add_node_filter_len);

		for(size_t node_type_idx = 0, count = lodge_node_type_get_count(); node_type_idx < count; node_type_idx++) {
			lodge_node_type_t node_type = lodge_node_type_get_index(node_type_idx);
			strview_t node_type_name = lodge_node_type_get_name(node_type);

			if(editor->add_node_filter_len == 0 || strview_begins_with(node_type_name, add_node_filter)) {
				if(nk_contextual_item_text(ctx, node_type_name.s, node_type_name.length, NK_TEXT_ALIGN_LEFT)) {
					lodge_graph_unconfigure(graph);
					{
						lodge_node_t node = lodge_graph_add_node(graph, node_type);
						lodge_node_id_t node_id = lodge_graph_get_node_id(graph, node);

						editor->node_bounds[node_id] = (struct nk_rect) {
							.x = mouse_pos_editor_space.x,
							.y = mouse_pos_editor_space.y,
							.w = -1,
							.h = -1,
						};

						if(node->outputs.count == 0) {
							lodge_graph_add_main(graph, node);
						}
					}
					lodge_graph_configure(graph);
				}
			}
		}

		nk_contextual_end(ctx);
	}

	//
	// Connection lines
	//
	for(size_t node_idx = 0; node_idx < nodes_count; node_idx++) {
		lodge_node_t node = lodge_graph_get_node_from_index(graph, node_idx);
		lodge_node_id_t node_id = lodge_graph_get_node_id(graph, node);

		// Input pin circles
		for(lodge_pin_idx_t pin_idx = 0, pins_count = node->inputs.count; pin_idx < pins_count; pin_idx++) {
			struct lodge_ns_editor_pin *editor_pin = &editor->node_pins[node_id].inputs[pin_idx];
			struct nk_rect pin_rect = nk_rect(editor_pin->pos.x - 4.0f, editor_pin->pos.y - 4.0f, 8.0f, 8.0f);
			nk_fill_circle(canvas, pin_rect, node_editor_pin_type_to_color(editor_pin->type));
		}

		// Output pin circles
		for(lodge_pin_idx_t pin_idx = 0, pins_count = node->outputs.count; pin_idx < pins_count; pin_idx++) {
			struct lodge_ns_editor_pin *editor_pin = &editor->node_pins[node_id].outputs[pin_idx];
			struct nk_rect pin_rect = nk_rect(editor_pin->pos.x - 4.0f, editor_pin->pos.y - 4.0f, 8.0f, 8.0f);
			nk_fill_circle(canvas, pin_rect, node_editor_pin_type_to_color(editor_pin->type));
		}

		// Lines
		for(lodge_pin_idx_t pin_idx = 0, pins_count = node->inputs.count; pin_idx < pins_count; pin_idx++) {
			struct lodge_pin_connection *link = &node->inputs.pins[pin_idx].connection;

			if(!link->node) {
				continue;
			}

			const lodge_node_id_t linked_node_id = lodge_graph_get_node_id(graph, link->node);
			struct lodge_ns_editor_pin *editor_in_pin = &editor->node_pins[node_id].inputs[pin_idx];
			struct lodge_ns_editor_pin *editor_out_pin = &editor->node_pins[linked_node_id].outputs[link->pin_index];

			nk_stroke_curve(canvas,
				editor_in_pin->pos.x, editor_in_pin->pos.y,
				editor_in_pin->pos.x - 50.0f, editor_in_pin->pos.y,
				editor_out_pin->pos.x + 50.0f, editor_out_pin->pos.y,
				editor_out_pin->pos.x, editor_out_pin->pos.y,
				2.0f,
				node_editor_pin_type_to_color(editor_in_pin->type)
			);
		}
	}
}


struct lodge_ns_editor* lodge_ns_editor_new(lodge_graph_t graph, lodge_ns_editor_on_save_func_t on_save, void *userdata)
{
	struct lodge_ns_editor *editor = calloc(1, lodge_ns_editor_sizeof());
	if(!editor) {
		return NULL;
	}
	lodge_ns_editor_new_inplace(editor, graph, on_save, userdata);
	return editor;
}

void lodge_ns_editor_free(struct lodge_ns_editor *editor)
{
	ASSERT(editor);
	lodge_ns_editor_free_inplace(editor);
	free(editor);
}

void lodge_ns_editor_new_inplace(struct lodge_ns_editor *editor, lodge_graph_t graph, lodge_ns_editor_on_save_func_t on_save, void *userdata)
{
	ASSERT(editor);
	ASSERT(graph);

	editor->modified = false;
	editor->graph = graph;
	editor->on_save = on_save;
	editor->userdata = userdata;

	for(size_t i = 0, count = LODGE_ARRAYSIZE(editor->node_bounds); i < count; i++) {
		editor->node_bounds[i] = (struct nk_rect) {
			.x = -1,
			.y = -1,
			.w = -1,
			.h = -1,
		};
	}
}

void lodge_ns_editor_free_inplace(struct lodge_ns_editor *editor)
{
}

size_t lodge_ns_editor_sizeof()
{
	return sizeof(struct lodge_ns_editor);
}

void lodge_ns_editor_update(struct lodge_ns_editor *editor, lodge_gui_t gui, float dt)
{
	node_editor_update(editor, gui, dt);
}
