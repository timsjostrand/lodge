#include "lodge_script_component.h"

#include "lodge_component_type.h"
#include "lodge_ns_graph.h"

void lodge_script_component_new_inplace(struct lodge_script_component *component, struct lodge_plugin_script *plugin)
{
	component->enabled = true;
	component->graph_asset = NULL;
}

void lodge_script_component_free_inplace(struct lodge_script_component *component, struct lodge_plugin_script *plugin)
{
	//if(component->graph_asset) {
	//	asset_release_dep(component->graph_asset);
	//}
}

lodge_component_type_t lodge_script_component_type_register(lodge_type_t lodge_type_graph_asset)
{
	return lodge_component_type_register((struct lodge_component_desc) {
		.name = strview_static("script"),
		.description = strview_static("Visual script."),
		.new_inplace = lodge_script_component_new_inplace,
		.free_inplace = lodge_script_component_free_inplace,
		.size = sizeof(struct lodge_script_component),
		.properties = {
			.count = 2,
			.elements = {
				{
					.name = strview_static("enabled"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_script_component, enabled),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
				{
					.name = strview_static("graph"),
					.type = lodge_type_graph_asset,
					.offset = offsetof(struct lodge_script_component, graph_asset)
				}
			}
		}
	});
}