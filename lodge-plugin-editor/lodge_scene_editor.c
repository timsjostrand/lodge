#include "lodge_scene_editor.h"

#include "math4.h"
#include "gruvbox.h"
#include "membuf.h"

#include "lodge_gui.h"
#include "lodge_gui_property_widget_factory.h"
#include "lodge_scene.h"
#include "lodge_scene_serialize.h"
#include "lodge_serialize_json.h"
#include "lodge_json.h"
#include "lodge_entity_type.h"
#include "lodge_system_type.h"
#include "lodge_component_type.h"
#include "lodge_variant.h"
#include "lodge_assert.h"
#include "lodge_file_picker_widget.h"
#include "lodge_window.h"

#include "lodge_plugin_editor.h"
#include "lodge_plugin_scenes.h"
#include "lodge_editor_selection_system.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define SCENE_EDITOR_ROW_HEIGHT 32

void lodge_scene_editor_new_inplace(struct lodge_scene_editor *editor, struct lodge_editor *owner)
{
	memset(editor, 0, sizeof(*editor));

	editor->editor = owner;

	lodge_entity_type_register((struct lodge_entity_type_desc) {
		.name = strview("Entity"),
		.components = {
			.count = 1,
			.elements = {
				lodge_component_types_find(strview("transform")),
			}
		}
	});
}

void lodge_scene_editor_free_inplace(struct lodge_scene_editor *editor)
{
}

size_t lodge_scene_editor_sizeof()
{
	return sizeof(struct lodge_scene_editor);
}

void lodge_scene_editor_impostor_on_modified(struct lodge_property *property, const void *object)
{
	//ASSERT_FAIL("Should not reach here");
}

struct lodge_component_type_property lodge_scene_editor_property_find_component_type(struct lodge_scene_editor *editor, struct lodge_property *property)
{
	for(size_t i = 0, count = editor->component_union.count; i < count; i++) {
		struct lodge_properties *properties = lodge_component_type_get_properties(editor->component_union.elements[i]);

		for(size_t j = 0, j_count = properties->count; j < j_count; j++) {
			struct lodge_property *other_property = &properties->elements[j];

			if(other_property->offset == property->offset
				&& other_property->type == property->type
				&& strview_equals(other_property->name, property->name)) {
				return (struct lodge_component_type_property) {
					.type = editor->component_union.elements[i],
					.property = &properties->elements[j],
				};
			}
		} 
	}
	return (struct lodge_component_type_property) { 0 };
}

bool lodge_scene_editor_impostor_pre_modified(struct lodge_property *property, const void *object, const void *dst, const void *src)
{
	struct lodge_scene_editor *scene_editor = (struct lodge_scene_editor *)property->userdata;
	ASSERT(scene_editor);

	struct lodge_component_type_property component_type = lodge_scene_editor_property_find_component_type(scene_editor, property);
	ASSERT(component_type.type && component_type.property);

	struct lodge_scene *scene = lodge_editor_get_current_scene(scene_editor->editor);

	for(size_t i = 0, count = scene_editor->selected_count; i < count; i++) {
		lodge_entity_t entity = scene_editor->selected[i];
		void *entity_component = lodge_scene_get_entity_component(scene, entity, component_type.type);

		// Old value
		char *prev_property_str = NULL;
		{
			lodge_json_t json = lodge_property_to_json(component_type.property, entity_component);
			prev_property_str = lodge_json_to_string(json);
			lodge_json_free(json);
		}

		lodge_property_set(component_type.property, entity_component, src);

		// Debugging -- undo?
		{
			lodge_json_t json = lodge_property_to_json(component_type.property, entity_component);
			char *new_property_str = lodge_json_to_string(json);
			lodge_json_free(json);

			strview_t component_name = lodge_component_type_get_name(component_type.type);
			strview_t property_name = component_type.property->name;
			printf(
				"Property modified: %" PRIxPTR "::" STRVIEW_PRINTF_FMT "::" STRVIEW_PRINTF_FMT " `%s` => `%s`\n",
				(uintptr_t)entity,
				STRVIEW_PRINTF_ARG(component_name),
				STRVIEW_PRINTF_ARG(property_name),
				prev_property_str,
				new_property_str
			);

			free(new_property_str);
		}
		free(prev_property_str);
	}

	return false;
}

bool lodge_scene_editor_is_entity_selected(struct lodge_scene_editor *editor, lodge_entity_t entity)
{
	return membuf_find(
		membuf_wrap(editor->selected),
		editor->selected_count,
		&entity,
		sizeof(lodge_entity_t)
	) >= 0;
}

void lodge_scene_editor_set_entity_selected(struct lodge_scene_editor *editor, lodge_entity_t entity, bool selected)
{
	int64_t index = membuf_find(
		membuf_wrap(editor->selected),
		editor->selected_count,
		&entity,
		sizeof(lodge_entity_t)
	);

	if(selected) {
		if(index < 0) {
			membuf_append(
				membuf_wrap(editor->selected),
				&editor->selected_count,
				&entity,
				sizeof(lodge_entity_t)
			);
		}
	} else {
		if(index >= 0) {
			membuf_delete(membuf_wrap(editor->selected), &editor->selected_count, index, 1);
		}
	}
}