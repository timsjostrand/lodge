#include "lodge_static_mesh_component.h"

#include "math4.h"
#include "strbuf.h"

#include "lodge_system_type.h"
#include "lodge_component_type.h"
#include "lodge_transform_component.h"
#include "lodge_scene.h"
#include "lodge_assets2.h"

#include <stdio.h>

lodge_component_type_t LODGE_COMPONENT_TYPE_STATIC_MESH = NULL;

static void lodge_static_mesh_component_new_inplace(struct lodge_static_mesh_component *component, void *userdata)
{
	*component = (struct lodge_static_mesh_component) { 0 };
}

static void lodge_static_mesh_component_free_inplace(struct lodge_static_mesh_component *component, void *userdata)
{
#if 0
	// TODO(TS): these should live in resource managers instead
	if(component->asset) {
		lodge_assets2_release( component->asset );
	}
#endif
}

lodge_component_type_t lodge_static_mesh_component_type_register(lodge_type_t static_mesh_asset_type, lodge_type_t shader_asset_type, lodge_type_t texture_asset_type)
{
	ASSERT(!LODGE_COMPONENT_TYPE_STATIC_MESH);

	if(!LODGE_COMPONENT_TYPE_STATIC_MESH) {
		LODGE_COMPONENT_TYPE_STATIC_MESH = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("static_mesh"),
			.description = strview_static("Adds a static mesh (without animation) that can be rendered in scene_render_system."),
			.new_inplace = &lodge_static_mesh_component_new_inplace,
			.free_inplace = &lodge_static_mesh_component_free_inplace,
			.size = sizeof(struct lodge_static_mesh_component),
			.properties = {
				.count = 3,
				.elements = {
					{
						.name = strview_static("static_mesh"),
						.type = static_mesh_asset_type,
						.offset = offsetof(struct lodge_static_mesh_component, fbx_asset),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						.name = strview_static("shader"),
						.type = shader_asset_type,
						.offset = offsetof(struct lodge_static_mesh_component, shader_asset),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
					{
						.name = strview_static("material"),
						.type = texture_asset_type,
						.offset = offsetof(struct lodge_static_mesh_component, texture_asset),
						.flags = LODGE_PROPERTY_FLAG_NONE,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_STATIC_MESH;
}