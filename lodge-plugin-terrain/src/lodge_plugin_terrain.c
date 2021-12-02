#include "lodge_plugin_terrain.h"

#include "lodge_terrain_system.h"
#include "lodge_terrain_component.h"
#include "lodge_foliage_component.h"

#include "lodge_plugins.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"
#include "lodge_plugin_textures.h"

enum lodge_plugin_idx
{
	PLUGIN_IDX_SHADERS,
	PLUGIN_IDX_TEXTURES,
	PLUGIN_IDX_SCENE_RENDERER,
	PLUGIN_IDX_OPTIONAL_DEBUG_DRAW,
	PLUGIN_IDX_OPTIONAL_EDITOR,
	PLUGIN_IDX_MAX,
};

struct lodge_ret lodge_plugin_terrain_new_inplace(struct lodge_plugin_terrain *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	plugin->shaders = dependencies[PLUGIN_IDX_SHADERS];
	plugin->textures = dependencies[PLUGIN_IDX_TEXTURES];
	plugin->plugin_scene_renderer = dependencies[PLUGIN_IDX_SCENE_RENDERER];

	plugin->plugin_debug_draw = dependencies[PLUGIN_IDX_OPTIONAL_DEBUG_DRAW];
	plugin->plugin_editor = dependencies[PLUGIN_IDX_OPTIONAL_EDITOR];

	struct texture_types texture_types = lodge_plugin_textures_get_types(plugin->textures);

	plugin->types = (struct lodge_terrain_types) {
		.terrain_system_type = lodge_terrain_system_type_register(plugin),
		.terrain_component_type = lodge_terrain_component_type_register(texture_types.texture_asset_type),
		.foliage_component_type = lodge_foliage_component_type_register(),
	};

	return lodge_success();
}

void lodge_plugin_terrain_free_inplace(struct lodge_plugin_terrain *plugin)
{
	// TODO(TS): release dependencies
}

struct lodge_terrain_types lodge_plugin_terrain_get_types(struct lodge_plugin_terrain *terrain)
{
	return terrain ? terrain->types : (struct lodge_terrain_types) { 0 };
}

LODGE_PLUGIN_IMPL(lodge_plugin_terrain)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_terrain),
		.name = strview_static("terrain"),
		.new_inplace = &lodge_plugin_terrain_new_inplace,
		.free_inplace = &lodge_plugin_terrain_free_inplace,
		.dependencies ={
			.count = PLUGIN_IDX_MAX,
			.elements = {
				[PLUGIN_IDX_SHADERS] = {
					.name = strview("shaders"),
				},
				[PLUGIN_IDX_TEXTURES] = {
					.name = strview("textures"),
				},
				[PLUGIN_IDX_SCENE_RENDERER] = {
					.name = strview("scene_renderer"),
				},
				[PLUGIN_IDX_OPTIONAL_DEBUG_DRAW] = {
					.name = strview("debug_draw"),
					.optional = true,
				},
				[PLUGIN_IDX_OPTIONAL_EDITOR] = {
					.name = strview("editor"),
					.optional = true,
				},
			}
		}
	};
}
