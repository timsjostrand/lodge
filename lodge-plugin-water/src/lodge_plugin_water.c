#include "lodge_plugin_water.h"

#include "lodge_water_system.h"
#include "lodge_water_component.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"

#include "lodge_plugin_shaders.h"
#include "lodge_plugin_textures.h"

#include "config.h"

struct lodge_ret lodge_plugin_water_new_inplace(struct lodge_plugin_water *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	plugin->shaders = lodge_plugins_depend(plugins, plugin, strview("shaders"));
	ASSERT(plugin->shaders);

	plugin->textures = lodge_plugins_depend(plugins, plugin, strview("textures"));
	ASSERT(plugin->textures);

	plugin->images = lodge_plugins_depend(plugins, plugin, strview("images"));
	ASSERT(plugin->images);

	plugin->scene_renderer = lodge_plugins_depend(plugins, plugin, strview("scene_renderer"));
	ASSERT(plugin->scene_renderer);

	struct shader_types shader_types = lodge_plugin_shaders_get_types(plugin->shaders);
	struct texture_types texture_types = lodge_plugin_textures_get_types(plugin->textures);

	plugin->types = (struct lodge_water_types) {
		.water_system_type = lodge_water_system_type_register(plugin, shader_types.shader_asset_type),
		.water_component_type = lodge_water_component_type_register(plugin, texture_types.texture_asset_type),
	};

	return lodge_success();
}

void lodge_plugin_water_free_inplace(struct lodge_plugin_water *plugin)
{
	// TODO(TS): release dependencies
}

struct lodge_water_types lodge_plugin_water_get_types(struct lodge_plugin_water *plugin)
{
	return plugin ? plugin->types : (struct lodge_water_types) { 0 };
}

LODGE_PLUGIN_IMPL(lodge_plugin_water)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_water),
		.name = strview("water"),
		.new_inplace = lodge_plugin_water_new_inplace,
		.free_inplace = lodge_plugin_water_free_inplace,
		.update = NULL,
		.render = NULL,
		.static_mounts = {
			.count = 1,
			.elements = {
				{
					.src_dir = strview(ASSETS_DIR),
					.dst_point = strview("/plugins/water"),
				}
			}
		},
	};
}
