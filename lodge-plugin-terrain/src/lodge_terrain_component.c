#include "lodge_terrain_component.h"

#include "lodge_component_type.h"

lodge_component_type_t LODGE_COMPONENT_TYPE_TERRAIN = NULL;

void lodge_terrain_component_new_inplace(struct lodge_terrain_component *terrain)
{
	terrain->chunks_x = 16;
	terrain->chunks_y = 16;
	
	terrain->chunk_size.x = 1.0f / terrain->chunks_x;
	terrain->chunk_size.y = 1.0f / terrain->chunks_y;
	terrain->chunk_size.z = 1.0f;

	terrain->chunks = calloc(terrain->chunks_x * terrain->chunks_y, sizeof(struct lodge_terrain_chunk));
	for(uint32_t y=0, y_max = terrain->chunks_y; y<y_max; y++) {
		for(int x=0, x_max = terrain->chunks_x; x<x_max; x++) {
			struct lodge_terrain_chunk *chunk = lodge_terrain_component_get_chunk(terrain, x, y);
			chunk->offset.x = -0.5f + x*terrain->chunk_size.x;
			chunk->offset.y = -0.5f + y*terrain->chunk_size.y;
			chunk->visible = 1;
		}
	}

	terrain->plane = calloc(1, lodge_tesselated_plane_sizeof());
	lodge_tesselated_plane_new_inplace(terrain->plane);

	terrain->heightmap = NULL;
	terrain->material = (struct lodge_static_material) {
		.albedo = NULL,
		.normal = NULL,
		.displacement = NULL,
		.metalness = NULL,
	};
}

void lodge_terrain_component_free_inplace(struct lodge_terrain_component *terrain)
{
	free(terrain->chunks);
}

lodge_component_type_t lodge_terrain_component_type_register()
{
	ASSERT(!LODGE_COMPONENT_TYPE_TERRAIN);

	if(!LODGE_COMPONENT_TYPE_TERRAIN) {
		LODGE_COMPONENT_TYPE_TERRAIN = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("terrain"),
			.description = strview_static("Render terrain from a height field."),
			.new_inplace = lodge_terrain_component_new_inplace,
			.free_inplace = lodge_terrain_component_free_inplace,
			.size = sizeof(struct lodge_terrain_component),
			.properties = {
				.count = 2,
				.elements = {
					{
						.name = strview_static("chunks_x"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_terrain_component, chunks_x),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("chunks_y"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_terrain_component, chunks_y),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_TERRAIN;
}