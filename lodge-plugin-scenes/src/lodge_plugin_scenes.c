#include "lodge_plugin_scenes.h"

#include "lodge_plugins.h"
#include "lodge_plugin_files.h"
#include "lodge_type_asset.h"
#include "lodge_assets2.h"

#include "lodge_scene.h"
#include "lodge_scene_serialize.h"
#include "lodge_system_type.h"
#include "lodge_transform_component.h"

enum lodge_scenes_userdata
{
	USERDATA_FILES,
	USERDATA_ASSET_TYPE,
	USERDATA_COMPONENT_TYPE_TRANSFORM,
};

static bool lodge_assets_scene_new_inplace(struct lodge_assets2 *scenes, strview_t name, lodge_asset_t id, struct lodge_scene *scene)
{
	struct lodge_assets2 *files = lodge_assets2_get_userdata(scenes, USERDATA_FILES);
	ASSERT(files);

	//
	// TODO(TS): reload callback
	//
	lodge_asset_t scene_asset = lodge_assets2_find_by_name(files, name);
	if(!scene_asset) {
		return false;
	}
	const struct lodge_asset_file *file = lodge_assets2_get(files, scene_asset);
	if(!file) {
		return false;
	}

	if(!lodge_scene_from_text_inplace(scene, strview_make(file->data, file->size))) {
		return false;
	}

	return true;
}

static bool lodge_assets_scene_new_default_inplace(struct lodge_assets2 *scenes, struct lodge_scene *scene)
{
	lodge_scene_new_inplace(scene);

	//
	// FIXME(TS): not very nice to impose the selection system on every created scene...
	//
	lodge_system_type_t editor_selection_system = lodge_system_type_find(strview("editor_selection_system"));
	if(editor_selection_system) {
		lodge_scene_add_system(scene, editor_selection_system);
	}
	
	//if(scene_renderer_type) {
	//	lodge_scene_add_system(scene, scene_renderer_type);
	//}

	return true;
}

static void lodge_assets_scene_free_inplace(struct lodge_assets2 *scenes, strview_t name, lodge_asset_t id, struct lodge_scene *scene)
{
	//struct lodge_assets *files = lodge_assets_get_userdata(scenes, USERDATA_FILES);
	//ASSERT(files);
	//lodge_assets_release_depend(images, name, (struct lodge_asset_handle) {
	//	.assets = assets,
	//	.id = id,
	//});

	lodge_scene_free_inplace(scene);
}

static struct lodge_ret lodge_plugin_scenes_new_inplace(struct lodge_assets2 *scenes, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	struct lodge_assets2 *files = lodge_plugins_depend(plugins, scenes, strview("files"));
	if(!files) {
		return lodge_error("Failed to find plugin `files`");
	}

	lodge_assets2_new_inplace(scenes, &(struct lodge_assets2_desc) {
		.name = strview("scenes"),
		.size = lodge_scene_sizeof(),
		.new_inplace = &lodge_assets_scene_new_inplace,
		.new_default_inplace = &lodge_assets_scene_new_default_inplace,
		.free_inplace = &lodge_assets_scene_free_inplace,
		.reload_inplace = NULL,
	} );

	lodge_assets2_set_userdata(scenes, USERDATA_FILES, files);

	{
		lodge_type_t scene_asset_type = lodge_type_register_asset(strview("scene"), scenes);
		lodge_assets2_set_userdata(scenes, USERDATA_ASSET_TYPE, scene_asset_type);
	}

	//
	// FIXME(TS): this isn't the correct owner of the transform component (lodge-entity should probably be loaded as plugin instead).
	//
	if(!LODGE_COMPONENT_TYPE_TRANSFORM) {
		lodge_assets2_set_userdata(scenes, USERDATA_COMPONENT_TYPE_TRANSFORM, lodge_transform_component_type_register());
	}

	return lodge_success();
}

static void lodge_plugin_scenes_free_inplace(struct lodge_assets2 *scenes)
{
	lodge_assets2_free_inplace(scenes);
}

struct lodge_plugin_desc lodge_plugin_scenes()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = lodge_assets2_sizeof(),
		.name = strview("scenes"),
		.new_inplace = &lodge_plugin_scenes_new_inplace,
		.free_inplace = &lodge_plugin_scenes_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

struct lodge_plugin_scenes_types lodge_plugin_scenes_get_asset_type(struct lodge_assets2 *scenes)
{
	return (struct lodge_plugin_scenes_types) {
		.scene_asset_type = lodge_assets2_get_userdata(scenes, USERDATA_ASSET_TYPE),
		.transform_component_type = lodge_assets2_get_userdata(scenes, USERDATA_COMPONENT_TYPE_TRANSFORM),
	};
}
