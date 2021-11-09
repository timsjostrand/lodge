#include "lodge_point_light_component.h"

#include "lodge_component_type.h"

lodge_component_type_t LODGE_COMPONENT_TYPE_POINT_LIGHT;

static void lodge_point_light_component_new_inplace(struct lodge_point_light_component *point_light, void *userdata)
{
	point_light->intensities = vec3_ones();
	point_light->attenuation = 0.01f;
	point_light->ambient_coefficient = 0.1f;
	point_light->cone_angle = 0.0f;
	point_light->cone_direction = vec3_zero();
}

lodge_component_type_t lodge_point_light_component_type_register()
{
	ASSERT(!LODGE_COMPONENT_TYPE_POINT_LIGHT);

	if(!LODGE_COMPONENT_TYPE_POINT_LIGHT) {
		LODGE_COMPONENT_TYPE_POINT_LIGHT = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("point_light"),
			.description = strview_static("A point light that will light the scene."),
			.size = sizeof(struct lodge_point_light_component),
			.new_inplace = lodge_point_light_component_new_inplace,
			.free_inplace = NULL,
			.properties = {
				.count = 5,
				.elements = {
					{
						.name = strview_static("intensities"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_point_light_component, intensities),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("attenuation"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_point_light_component, attenuation),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("ambient_coefficient"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_point_light_component, ambient_coefficient),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("cone_angle"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_point_light_component, cone_angle),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("cone_direction"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_point_light_component, cone_direction),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_POINT_LIGHT;
}
