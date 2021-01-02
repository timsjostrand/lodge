#include "lodge_water_component.h"

#include "lodge_component_type.h"

lodge_component_type_t LODGE_COMPONENT_TYPE_WATER = NULL;

void lodge_water_component_new_inplace(struct lodge_water_component *water)
{
	water->wave_scale = vec3_make(100.0f, 100.0f, 0.1f);
	water->max_depth = 8.0f;
	water->color = vec3_make(0.0f, 0.125f, 0.20f);
}

void lodge_water_component_free_inplace(struct lodge_water_component *water)
{
}

lodge_component_type_t lodge_water_component_type_register()
{
	ASSERT(!LODGE_COMPONENT_TYPE_WATER);

	if(!LODGE_COMPONENT_TYPE_WATER) {
		LODGE_COMPONENT_TYPE_WATER = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("water"),
			.description = strview_static("A water plane."),
			.new_inplace = lodge_water_component_new_inplace,
			.free_inplace = lodge_water_component_free_inplace,
			.size = sizeof(struct lodge_water_component),
			.properties = {
				.count = 3,
				.elements = {
					{
						.name = strview_static("wave_scale"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_water_component, wave_scale),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("max_depth"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_water_component, max_depth),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("color"),
						.type = LODGE_TYPE_VEC3_COLOR,
						.offset = offsetof(struct lodge_water_component, color),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_WATER;
}