#include "lodge_water_component.h"

#include "lodge_plugin_water.h"
#include "lodge_assets2.h"
#include "lodge_component_type.h"

lodge_component_type_t LODGE_COMPONENT_TYPE_WATER = NULL;

void lodge_water_component_new_inplace(struct lodge_water_component *water, struct lodge_plugin_water *plugin)
{
	water->wave_scale = vec3_make(100.0f, 100.0f, 0.1f);
	water->max_depth = 8.0f;
	water->color = vec3_make(0.0f, 0.125f, 0.20f);

	water->foam_asset = lodge_assets2_register(plugin->textures, strview("plugins/water/water_foam.png"));
	water->normals_1_asset = lodge_assets2_register(plugin->textures, strview("plugins/water/water_normals1.png"));
	water->normals_2_asset = lodge_assets2_register(plugin->textures, strview("plugins/water/water_normals2.png"));
}

void lodge_water_component_free_inplace(struct lodge_water_component *water, struct lodge_plugin_water *plugin)
{
}

lodge_component_type_t lodge_water_component_type_register(struct lodge_plugin_water *plugin, lodge_type_t texture_asset_type)
{
	ASSERT(!LODGE_COMPONENT_TYPE_WATER);

	if(!LODGE_COMPONENT_TYPE_WATER) {
		LODGE_COMPONENT_TYPE_WATER = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview("water"),
			.description = strview("A water plane."),
			.new_inplace = lodge_water_component_new_inplace,
			.free_inplace = lodge_water_component_free_inplace,
			.size = sizeof(struct lodge_water_component),
			.userdata = plugin,
			.properties = {
				.count = 6,
				.elements = {
					{
						.name = strview("wave_scale"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_water_component, wave_scale),
					},
					{
						.name = strview("max_depth"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_water_component, max_depth),
					},
					{
						.name = strview("color"),
						.type = LODGE_TYPE_VEC3_COLOR,
						.offset = offsetof(struct lodge_water_component, color),
					},
					{
						.name = strview("foam"),
						.type = texture_asset_type,
						.offset = offsetof(struct lodge_water_component, foam_asset),
					},
					{
						.name = strview("normals_1"),
						.type = texture_asset_type,
						.offset = offsetof(struct lodge_water_component, normals_1_asset),
					},
					{
						.name = strview("normals_2"),
						.type = texture_asset_type,
						.offset = offsetof(struct lodge_water_component, normals_2_asset),
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_WATER;
}