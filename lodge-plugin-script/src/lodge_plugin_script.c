#include "lodge_plugin_script.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"

#include "lodge_assets2.h"
#include "lodge_type_asset.h"
#include "lodge_graphs.h"
#include "lodge_ns_node_types_default.h"
#include "lodge_ns_editor.h"
#include "lodge_plugin_editor.h"

#include "lodge_script_system.h"
#include "lodge_script_component.h"

struct lodge_plugin_script
{
	struct lodge_assets2		*graphs;
	struct lodge_script_types	types;
};

enum userdata
{
	USERDATA_EDITOR,
};

static void node_editor_update_glue(struct lodge_ns_editor *ns_editor, lodge_gui_t gui, struct lodge_editor *editor, float dt)
{
	lodge_ns_editor_update(ns_editor, gui, dt);
}

static void lodge_graph_asset_edit(struct lodge_assets2 *graphs, lodge_asset_t asset)
{
	struct lodge_graph *graph = lodge_assets2_get(graphs, asset);
	ASSERT_OR(graph) { return; }

	struct lodge_editor *editor = lodge_assets2_get_userdata(graphs, USERDATA_EDITOR);
	ASSERT_OR(editor) { return; }

	struct lodge_ns_editor *graph_editor = lodge_ns_editor_new(graph);

	lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
		.name = strview("Graph"),
		.panel = graph_editor,
		.pos = vec2_make(0, 0),
		.size = vec2_make(640, 480),
		.update = &node_editor_update_glue,
		.hide_scrollbar = true,
		.allow_instances = true,
	});
}

struct lodge_ret lodge_plugin_script_new_inplace(struct lodge_plugin_script *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	lodge_node_types_default_register();

	{
		struct lodge_editor *editor = lodge_plugins_depend(plugins, plugin, strview_static("editor"));
		if(!editor) {
			return lodge_error("Failed to find plugin `editor`");
		}

		plugin->graphs = calloc(1, lodge_assets2_sizeof());
		lodge_graphs_new_inplace(plugin->graphs);

		lodge_assets2_set_userdata(plugin->graphs, USERDATA_EDITOR, editor);

		plugin->types.graph_asset_type = lodge_type_register_asset(strview_static("graph_asset"), plugin->graphs);

		lodge_type_asset_set_edit_func(plugin->types.graph_asset_type, &lodge_graph_asset_edit);
	}

	plugin->types.script_system_type = lodge_script_system_type_register(plugin);
	plugin->types.script_component_type = lodge_script_component_type_register(plugin->types.graph_asset_type);

	return lodge_success();
}

void lodge_plugin_script_free_inplace(struct lodge_plugin_script *plugin)
{
	lodge_graphs_free_inplace(plugin->graphs);
}

struct lodge_script_types lodge_plugin_script_get_types(struct lodge_plugin_script *plugin)
{
	return plugin ? plugin->types : (struct lodge_script_types) { 0 };
}

LODGE_PLUGIN_IMPL(lodge_plugin_script)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_script),
		.name = strview_static("script"),
		.new_inplace = &lodge_plugin_script_new_inplace,
		.free_inplace = &lodge_plugin_script_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

