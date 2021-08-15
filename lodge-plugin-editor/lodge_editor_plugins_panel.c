#include "lodge_editor_plugins_panel.h"

#include "lodge_plugin.h"
#include "lodge_plugins.h"
#include "lodge_gui.h"

struct lodge_plugins_panel
{
	vec2 pos;
	vec2 size;
};

void lodge_plugins_panel_new_inplace(struct lodge_plugins_panel *panel)
{
	*panel = (struct lodge_plugins_panel) {
		.pos = vec2_zero(),
		.size = vec2_make(100.0f, 100.0f),
	};
}

void lodge_plugins_panel_free_inplace(struct lodge_plugins_panel *panel)
{
}

size_t lodge_plugins_panel_sizeof()
{
	return sizeof(struct lodge_plugins_panel);
}

void lodge_plugins_panel_gui_update(struct lodge_plugins_panel *panel, lodge_gui_t gui, struct lodge_plugin_editor *editor, struct lodge_plugins *plugins)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	//
	// TODO(TS): nk_begin and nk_end should be handled by lodge_editor_panels()
	//
	if(nk_begin(ctx, "Plugins", nk_rect(xy_of(panel->pos), xy_of(panel->size)), NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)) {
		nk_layout_row_dynamic(ctx, 0, 2);
		for(uint32_t i = 0, count = lodge_plugins_get_count(plugins); i < count; i++) {
			const struct lodge_plugin_desc *desc = lodge_plugins_get_desc(plugins, i);

			bool is_dependency = lodge_plugins_is_dependency(plugins, editor, desc->name);
			if(nk_checkbox_text(ctx, desc->name.s, desc->name.length, &is_dependency)) {

				if(!is_dependency) {
					const bool ret = lodge_plugins_undepend(plugins, editor, desc->name);
					ASSERT(ret);
				} else {
					const bool ret = lodge_plugins_depend(plugins, editor, desc->name);
					ASSERT(ret);
				}
			}

			const uint32_t deps_count = lodge_plugins_get_dependencies_count(plugins, i);
			nk_labelf(ctx, NK_TEXT_LEFT, "%u", deps_count);
		}

		nk_end(ctx);
	}
}
