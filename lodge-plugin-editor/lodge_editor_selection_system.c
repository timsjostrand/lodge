#include "lodge_editor_selection_system.h"

#include "lodge_properties.h"
#include "lodge_system_type.h"
#include "lodge_scene.h"

#include "sparse_set.h"

struct lodge_editor_selection_system
{
	sparse_set_t selected;
	uint32_t count;
};

static void lodge_editor_selection_system_new_inplace(struct lodge_editor_selection_system *system)
{
	system->selected = sparse_set_new(sizeof(bool), 256, 4096);
	system->count = 0;
}

lodge_system_type_t LODGE_SYSTEM_TYPE_EDITOR_SELECTION = NULL;

static void lodge_editor_selection_system_update(struct lodge_editor_selection_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt)
{
	system->count = sparse_set_get_dense_count(system->selected);
}

//
// TODO(TS): this should be a generic helper function
//
static size_t lodge_entity_to_sparse_index(lodge_entity_t entity)
{
	ASSERT(entity);
	return (size_t)((uintptr_t)entity - 1);
}

lodge_system_type_t lodge_editor_selection_system_type_register(struct lodge_plugin_editor *plugin)
{
	if(!LODGE_SYSTEM_TYPE_EDITOR_SELECTION) {
		LODGE_SYSTEM_TYPE_EDITOR_SELECTION = lodge_system_type_register((struct lodge_system_type_desc) {
			.name = strview_static("editor_selection_system"),
			.size = sizeof(struct lodge_editor_selection_system),
			.new_inplace = lodge_editor_selection_system_new_inplace,
			.free_inplace = NULL,
			.update = lodge_editor_selection_system_update,
			.plugin = plugin,
			.properties = {
				.count = 1,
				.elements = {
					{
						.name = strview_static("count"),
						.type = LODGE_TYPE_U32,
						.offset = offsetof(struct lodge_editor_selection_system, count),
						.flags = LODGE_PROPERTY_FLAG_READ_ONLY,
					}
				}
			}
		});
	}

	return LODGE_SYSTEM_TYPE_EDITOR_SELECTION;
}

void lodge_scene_set_entity_selected(lodge_scene_t scene, lodge_entity_t entity, bool selected)
{
	ASSERT_OR(LODGE_SYSTEM_TYPE_EDITOR_SELECTION && scene && entity) { return; }

	struct lodge_editor_selection_system *system = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_EDITOR_SELECTION);
	ASSERT_OR(system) { return; }

	//
	// TODO(TS): should _remove() when selected==false, to make iteration easier and keep `dense` compact
	//
	const uint32_t index = lodge_entity_to_sparse_index(entity);
	sparse_set_set(system->selected, index, &selected);
}

bool lodge_scene_is_entity_selected(lodge_scene_t scene, lodge_entity_t entity)
{
	ASSERT_OR(LODGE_SYSTEM_TYPE_EDITOR_SELECTION && scene && entity) { return false; }

	struct lodge_editor_selection_system *system = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_EDITOR_SELECTION);
	ASSERT_OR(system) { return false; }

	const uint32_t index = lodge_entity_to_sparse_index(entity);
	bool* selected = sparse_set_get(system->selected, index);
	return selected ? *selected : false;
}

lodge_entity_t* lodge_scene_selected_it_begin(lodge_scene_t scene)
{
	ASSERT_OR(LODGE_SYSTEM_TYPE_EDITOR_SELECTION && scene) { return NULL; }

	struct lodge_editor_selection_system *system = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_EDITOR_SELECTION);
	ASSERT_OR(system) { return NULL; }

	return sparse_set_it_begin(system->selected);
}

lodge_entity_t* lodge_scene_selected_it_next(lodge_scene_t scene, lodge_entity_t *it)
{
	ASSERT_OR(LODGE_SYSTEM_TYPE_EDITOR_SELECTION && scene) { return NULL; }

	struct lodge_editor_selection_system *system = lodge_scene_get_system(scene, LODGE_SYSTEM_TYPE_EDITOR_SELECTION);
	ASSERT_OR(system) { return NULL; }

	return sparse_set_it_next(system->selected, it);
}
