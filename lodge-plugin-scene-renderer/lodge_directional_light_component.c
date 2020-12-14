#include "lodge_directional_light_component.h"

#include "lodge_component_type.h"

lodge_component_type_t LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT;

static void lodge_directional_light_component_new_inplace(struct lodge_directional_light_component *directional_light)
{
	directional_light->dir = vec3_make(0.0f, 0.0f, -1.0f);
	directional_light->intensities = vec3_ones();
}

lodge_component_type_t lodge_directional_light_component_type_register()
{
	ASSERT(!LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT);

	// TODO(TS): should the entity rotation just be the dir?

	if(!LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT) {
		LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("directional_light"),
			.description = strview_static("A directional light that will light the scene (only 1 supported at the moment)."),
			.size = sizeof(struct lodge_directional_light_component),
			.new_inplace = lodge_directional_light_component_new_inplace,
			.free_inplace = NULL,
			.properties = {
				.count = 2,
				.elements = {
					{
						.name = strview_static("dir"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_directional_light_component, dir),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("intensities"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_directional_light_component, intensities),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT;
}

#include "lodge_scene.h"

vec3 lodge_directional_light_get_dir(lodge_scene_t scene)
{
	lodge_scene_components_foreach(scene, struct lodge_directional_light_component*, directional_light, LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT) {
		return directional_light->dir;
	}
	return vec3_zero();
}
