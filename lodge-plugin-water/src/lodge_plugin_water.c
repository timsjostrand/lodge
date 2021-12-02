#include "lodge_plugin_water.h"

#include "lodge_water_system.h"
#include "lodge_water_component.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"

#include "lodge_plugin_shaders.h"
#include "lodge_plugin_textures.h"

#include "config.h"

enum lodge_plugin_idx
{
	PLUGIN_IDX_SHADERS,
	PLUGIN_IDX_TEXTURES,
	PLUGIN_IDX_IMAGES,
	PLUGIN_IDX_SCENE_RENDERER,
	PLUGIN_IDX_MAX,
};

struct lodge_ret lodge_plugin_water_new_inplace(struct lodge_plugin_water *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	plugin->shaders = dependencies[PLUGIN_IDX_SHADERS];
	plugin->textures = dependencies[PLUGIN_IDX_TEXTURES];
	plugin->images = dependencies[PLUGIN_IDX_IMAGES];
	plugin->scene_renderer = dependencies[PLUGIN_IDX_SCENE_RENDERER];

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
		.new_inplace = &lodge_plugin_water_new_inplace,
		.free_inplace = &lodge_plugin_water_free_inplace,
		.static_mounts = {
			.count = 1,
			.elements = {
				{
					.src_dir = strview(ASSETS_DIR),
					.dst_point = strview("/plugins/water"),
				}
			}
		},
		.dependencies = {
			.count = PLUGIN_IDX_MAX,
			.elements = {
				[PLUGIN_IDX_SHADERS] = {
					.name = strview("shaders"),
				},
				[PLUGIN_IDX_TEXTURES] = {
					.name = strview("textures"),
				},
				[PLUGIN_IDX_IMAGES] = {
					.name = strview("images"),
				},
				[PLUGIN_IDX_SCENE_RENDERER] = {
					.name = strview("scene_renderer"),
				},
			}
		}
	};
}
