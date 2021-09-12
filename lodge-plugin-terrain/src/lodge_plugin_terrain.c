#include "lodge_plugin_terrain.h"

#include "lodge_terrain_system.h"
#include "lodge_terrain_component.h"
#include "lodge_foliage_component.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"

struct lodge_ret lodge_plugin_terrain_new_inplace(struct lodge_plugin_terrain *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	plugin->shaders = lodge_plugins_depend(plugins, plugin, strview_static("shaders"));
	ASSERT(plugin->shaders);

	plugin->plugin_scene_renderer = lodge_plugins_depend(plugins, plugin, strview_static("scene_renderer"));
	ASSERT(plugin->plugin_scene_renderer);

	// FIXME(TS): support optional dependencies
	plugin->plugin_debug_draw = lodge_plugins_depend(plugins, plugin, strview_static("debug_draw"));

	plugin->types = (struct lodge_terrain_types) {
		.terrain_system_type = lodge_terrain_system_type_register(plugin),
		.terrain_component_type = lodge_terrain_component_type_register(),
		.foliage_component_type = lodge_foliage_component_type_register(),
	};

	return lodge_success();
}

void lodge_plugin_terrain_free_inplace(struct lodge_plugin_terrain *plugin)
{
	// TODO(TS): release dependencies
}

struct lodge_plugin_desc lodge_plugin_terrain()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_terrain),
		.name = strview_static("terrain"),
		.new_inplace = lodge_plugin_terrain_new_inplace,
		.free_inplace = lodge_plugin_terrain_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

struct lodge_terrain_types lodge_plugin_terrain_get_types(struct lodge_plugin_terrain *terrain)
{
	return terrain ? terrain->types : (struct lodge_terrain_types) { 0 };
}
