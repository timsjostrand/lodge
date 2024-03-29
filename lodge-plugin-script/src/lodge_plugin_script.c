#include "lodge_plugin_script.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"

#include "lodge_assets2.h"
#include "lodge_type_asset.h"
#include "lodge_graphs.h"
#include "lodge_ns_node_types_default.h"
#include "lodge_ns_editor.h"
#include "lodge_ns_graph_serialize.h"
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

enum lodge_plugin_idx
{
	PLUGIN_IDX_EDITOR,
	PLUGIN_IDX_MAX,
};

static void node_editor_update_glue(struct lodge_ns_editor *ns_editor, lodge_gui_t gui, struct lodge_editor *editor, float dt)
{
	lodge_ns_editor_update(ns_editor, gui, dt);
}

static void node_editor_on_close_glue(struct lodge_ns_editor *ns_editor, lodge_gui_t gui, struct lodge_editor *editor)
{
	lodge_ns_editor_free_inplace(ns_editor);
	free(ns_editor);
}

static bool node_editor_on_save(struct lodge_ns_editor *ns_editor, lodge_graph_t graph, struct lodge_assets2 *graphs)
{
	size_t text_size = 0;
	char *text = lodge_graph_to_text(graph, &text_size);
	if(!text) {
		return false;
	}

	ASSERT_FAIL("TODO: write serialized text to disk");

	//
	// TODO(TS): figure out where on disk the asset goes to be loaded properly from VFS
	// 
	// Write `text` => file
	// 
	// TODO(TS): does the asset also contain editor meta data (positions etc)?
	// 
	// Probably most rebust to write both at the same time, but to two separate files (for easy diffing).
	//
	//lodge_assets2_get_name(graphs, graph);

	free(text);

	return true;
}

static void lodge_graph_asset_edit(struct lodge_assets2 *graphs, lodge_asset_t asset)
{
	struct lodge_graph *graph = lodge_assets2_get(graphs, asset);
	ASSERT_OR(graph) { return; }

	struct lodge_editor *editor = lodge_assets2_get_userdata(graphs, USERDATA_EDITOR);
	ASSERT_OR(editor) { return; }

	struct lodge_ns_editor *graph_editor = lodge_ns_editor_new(graph, &node_editor_on_save, graphs);

	strview_t graph_name = lodge_assets2_get_name(graphs, asset);

	lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
		.name = graph_name,
		.panel = graph_editor,
		.pos = vec2_make(0, 0),
		.size = vec2_make(640, 480),
		.update = &node_editor_update_glue,
		.on_close = &node_editor_on_close_glue,
		.hide_scrollbar = true,
		.allow_instances = true,
	});
}

struct lodge_ret lodge_plugin_script_new_inplace(struct lodge_plugin_script *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	lodge_node_types_default_register();

	plugin->graphs = calloc(1, lodge_assets2_sizeof());
	lodge_graphs_new_inplace(plugin->graphs);

	plugin->types.graph_asset_type = lodge_type_register_asset(strview("graph_asset"), plugin->graphs);

	//
	// Optionally enable graph editing.
	//
	struct lodge_editor *editor = dependencies[PLUGIN_IDX_EDITOR];
	if(editor) {
		lodge_assets2_set_userdata(plugin->graphs, USERDATA_EDITOR, editor);
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
		.name = strview("script"),
		.new_inplace = &lodge_plugin_script_new_inplace,
		.free_inplace = &lodge_plugin_script_free_inplace,
		.update = NULL,
		.render = NULL,
		.dependencies ={
			.count = 1,
			.elements = {
				[PLUGIN_IDX_EDITOR] = {
					.name = strview("editor"),
					.optional = true,
				}
			}
		}
	};
}

