#include "lodge_plugin_editor.h"

#include "lodge_editor_controller.h"
#include "lodge_editor_selection_system.h"

struct lodge_plugin_editor
{
	struct lodge_editor_types types;
};

struct lodge_ret lodge_plugin_editor_new_inplace(struct lodge_plugin_editor *plugin, struct lodge_plugins *plugins)
{
	plugin->types.selection_system_type = lodge_editor_selection_system_type_register(plugin);

	plugin->types.controller_component_type = lodge_editor_controller_component_type_register();
	plugin->types.controller_system_type = lodge_editor_controller_system_type_register(plugin);

	return lodge_ret_make_success();
}

struct lodge_editor_types lodge_plugin_editor_get_types(struct lodge_plugin_editor *editor)
{
	return editor->types;
}

struct lodge_plugin_desc lodge_plugin_editor()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_editor),
		.name = strview_static("editor"),
		.new_inplace = &lodge_plugin_editor_new_inplace,
		.free_inplace = NULL,
		.update = NULL,
		.render = NULL,
	};
}
