#include "lodge_editor_viewport_panel.h"

#include "lodge_gui.h"
#include "lodge_plugin_editor.h"
#include "lodge_texture.h"
#include "lodge_framebuffer.h"
#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_bound_func.h"

#include "lodge_editor_selection_system.h"

struct lodge_viewport_panel
{
	uint32_t				last_w;
	uint32_t				last_h;
	lodge_texture_t			texture;
	lodge_framebuffer_t		framebuffer;
	uint32_t				gui_image_index;
};

size_t lodge_editor_viewport_panel_sizeof()
{
	return sizeof(struct lodge_viewport_panel);
}

static void lodge_editor_viewport_panel_resize(struct lodge_viewport_panel *panel, uint32_t width, uint32_t height)
{
	width = max(width, 1);
	height = max(height, 1);

	panel->last_w = width;
	panel->last_h = height;
	
	if(panel->texture) {
		lodge_texture_reset(panel->texture);
	}
	panel->texture = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = 1,
		.texture_format = LODGE_TEXTURE_FORMAT_RGB8,
	});
	
	if(panel->framebuffer) {
		lodge_framebuffer_reset(panel->framebuffer);
	}
	panel->framebuffer = lodge_framebuffer_make(&(struct lodge_framebuffer_desc) {
		.colors_count = 1,
		.colors = {
			panel->texture,
		}
	});
}

void lodge_editor_viewport_panel_new_inplace(struct lodge_viewport_panel *panel, struct lodge_plugins *plugins)
{
	lodge_editor_viewport_panel_resize(panel, 1920, 1080);
}

void lodge_editor_viewport_panel_update(struct lodge_viewport_panel *panel, struct lodge_gui *gui, struct lodge_editor *editor, float dt)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	//enum nk_panel_flags window_flags = NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_NO_SCROLLBAR;
	//if(nk_begin(ctx, "Viewport", nk_rect(300, 0, 1920-300-500, 1080), window_flags)) {
    struct nk_rect panel_rect = nk_window_get_content_region(ctx);
	nk_layout_row_static(ctx, panel_rect.h, panel_rect.w, 1);

	if(panel_rect.w != panel->last_w || panel_rect.h != panel->last_h) {
		lodge_editor_viewport_panel_resize(panel, panel_rect.w, panel_rect.h);
	}

	// Render scene
	{
		struct lodge_scene *scene = lodge_editor_get_current_scene(editor);
		if(scene) {
			lodge_scene_render(
				scene,
				&(struct lodge_system_render_params) {
					.window_rect = (struct lodge_recti) {
						.x0 = 0,
						.y0 = 0,
						.x1 = panel_rect.w,
						.y1 = panel_rect.h
					},
					.framebuffer = panel->framebuffer
			});

			if(nk_widget_has_mouse_click_down(ctx, NK_BUTTON_RIGHT, true)) {
				struct lodge_editor_types editor_types = lodge_editor_get_types(editor);
				lodge_scene_components_foreach(scene, struct lodge_editor_controller_component*, editor_controller, editor_types.controller_component_type) {
					lodge_editor_set_controller(editor, editor_controller);
					break;
				}
			} else {
				lodge_editor_set_controller(editor, NULL);
			}

			//
			// Check if entity was picked
			//
			lodge_entity_t picked_entity = NULL;
			if(nk_widget_is_mouse_clicked(ctx, NK_BUTTON_LEFT)) {
				struct nk_vec2 mouse_pos = ctx->input.mouse.pos;

				struct lodge_scene_funcs *scene_funcs = lodge_scene_get_funcs(scene);
				if(lodge_bound_func_is_set(scene_funcs->get_entity_at_screen_pos)) {
					picked_entity = lodge_bound_func_call(scene_funcs->get_entity_at_screen_pos, vec2_make(mouse_pos.x - panel_rect.x, mouse_pos.y - panel_rect.y));
				}
				
				if(picked_entity) {
					strview_t picked_entity_name = lodge_scene_get_entity_name(scene, picked_entity);
					printf(STRVIEW_PRINTF_FMT "\n", STRVIEW_PRINTF_ARG(picked_entity_name));
			
					const bool was_selected = lodge_scene_is_entity_selected(scene, picked_entity);
					lodge_scene_set_entity_selected(scene, picked_entity, !was_selected);
				}
			}
		}
	}

	struct lodge_scene *scene = lodge_editor_get_current_scene(editor);
	nk_image(ctx, nk_image_ptr(panel->texture));

	//	nk_end(ctx);
	//}
}
