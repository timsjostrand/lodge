#include "lodge_billboard_component.h"

#include "lodge_component_type.h"

static void lodge_billboard_component_new_inplace(struct lodge_billboard_component *billboard)
{
	billboard->size = vec2_make(32.0f, 32.0f);
	billboard->editor_only = true;
	billboard->fixed_size = true;
	billboard->texture_asset = NULL;
}

lodge_component_type_t lodge_billboard_component_type_register(lodge_type_t texture_asset_type)
{
	return lodge_component_type_register((struct lodge_component_desc) {
		.name = strview_static("billboard"),
		.description = strview_static("Sprite that is always facing the camera."),
		.size = sizeof(struct lodge_billboard_component),
		.new_inplace = lodge_billboard_component_new_inplace,
		.free_inplace = NULL,
		.properties = {
			.count = 4,
			.elements = {
				{
					.name = strview_static("size"),
					.type = LODGE_TYPE_VEC2,
					.offset = offsetof(struct lodge_billboard_component, size),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
				{
					.name = strview_static("fixed_size"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_billboard_component, fixed_size),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
				{
					.name = strview_static("editor_only"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_billboard_component, editor_only),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
				{
					.name = strview_static("texture"),
					.type = texture_asset_type,
					.offset = offsetof(struct lodge_billboard_component, texture_asset),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
			}
		}
	});
}
