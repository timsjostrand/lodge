#include "lodge_editor_plugins_panel.h"

#include "lodge_plugin.h"
#include "lodge_plugins.h"
#include "lodge_gui.h"

struct lodge_plugins_panel
{
	struct lodge_plugins *plugins;
};

void lodge_plugins_panel_new_inplace(struct lodge_plugins_panel *panel, struct lodge_plugins *plugins)
{
	*panel = (struct lodge_plugins_panel) {
		.plugins = plugins,
	};
}

void lodge_plugins_panel_free_inplace(struct lodge_plugins_panel *panel)
{
	//
	// TODO(TS): undepend on everything we loaded
	//
}

size_t lodge_plugins_panel_sizeof()
{
	return sizeof(struct lodge_plugins_panel);
}

void lodge_plugins_panel_update(struct lodge_plugins_panel *panel, lodge_gui_t gui, struct lodge_editor *editor, float dt)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	nk_layout_row_dynamic(ctx, 0, 2);

	for(uint32_t i = 0, count = lodge_plugins_get_count(panel->plugins); i < count; i++) {
		const struct lodge_plugin_desc *desc = lodge_plugins_get_desc(panel->plugins, i);

		bool is_dependency = lodge_plugins_is_dependency(panel->plugins, editor, desc->name);
		if(nk_checkbox_text(ctx, desc->name.s, desc->name.length, &is_dependency)) {
			if(!is_dependency) {
				const bool ret = lodge_plugins_undepend(panel->plugins, editor, desc->name);
				ASSERT(ret);
				LODGE_UNUSED(ret);
			} else {
				const bool ret = lodge_plugins_depend(panel->plugins, editor, desc->name);
				ASSERT(ret);
				LODGE_UNUSED(ret);
			}
		}

		const uint32_t deps_count = lodge_plugins_get_dependencies_count(panel->plugins, i);
		nk_labelf(ctx, NK_TEXT_LEFT, "%u", deps_count);
	}
}

void lodge_stats_panel_update(struct lodge_plugins_panel *panel, lodge_gui_t gui, struct lodge_editor *editor, float dt)
{
	struct nk_context *ctx = lodge_gui_to_ctx(gui);

	nk_layout_row_dynamic(ctx, 0, 2);

	struct lodge_plugins_frame_times frame_times = lodge_plugins_get_frame_times(panel->plugins);
	NK_UNUSED(frame_times);
		
	nk_label(ctx, "FPS: ", NK_TEXT_LEFT);
	nk_labelf(ctx, NK_TEXT_LEFT, "%d", frame_times.frames);
		
	nk_label(ctx, "Frame-time, avg: ", NK_TEXT_LEFT);
	nk_labelf(ctx, NK_TEXT_LEFT, "%.1f ms", frame_times.frame_time_avg);
		
	nk_label(ctx, "Frame-time, min: ", NK_TEXT_LEFT);
	nk_labelf(ctx, NK_TEXT_LEFT, "%.1f ms", frame_times.frame_time_min);
		
	nk_label(ctx, "Frame-time, max: ", NK_TEXT_LEFT);
	nk_labelf(ctx, NK_TEXT_LEFT, "%.1f ms", frame_times.frame_time_max);
}
