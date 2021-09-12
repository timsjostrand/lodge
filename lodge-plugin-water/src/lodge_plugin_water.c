#include "lodge_plugin_water.h"

#include "lodge_water_system.h"
#include "lodge_water_component.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"

struct lodge_ret lodge_plugin_water_new_inplace(struct lodge_plugin_water *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	plugin->shaders = lodge_plugins_depend(plugins, plugin, strview_static("shaders"));
	ASSERT(plugin->shaders);

	plugin->textures = lodge_plugins_depend(plugins, plugin, strview_static("textures"));
	ASSERT(plugin->textures);

	plugin->images = lodge_plugins_depend(plugins, plugin, strview_static("images"));
	ASSERT(plugin->images);

	plugin->scene_renderer = lodge_plugins_depend(plugins, plugin, strview_static("scene_renderer"));
	ASSERT(plugin->scene_renderer);

	plugin->types = (struct lodge_water_types) {
		.water_system_type = lodge_water_system_type_register(plugin),
		.water_component_type = lodge_water_component_type_register(),
	};

	return lodge_success();
}

void lodge_plugin_water_free_inplace(struct lodge_plugin_water *plugin)
{
	// TODO(TS): release dependencies
}

struct lodge_plugin_desc lodge_plugin_water()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_water),
		.name = strview_static("water"),
		.new_inplace = lodge_plugin_water_new_inplace,
		.free_inplace = lodge_plugin_water_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

struct lodge_water_types lodge_plugin_water_get_types(struct lodge_plugin_water *plugin)
{
	return plugin ? plugin->types : (struct lodge_water_types) { 0 };
}
