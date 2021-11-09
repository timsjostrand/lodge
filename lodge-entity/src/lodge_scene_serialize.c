#include "lodge_scene_serialize.h"

#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_component_type.h"
#include "lodge_entity_type.h" // for lodge_entity_components_desc

#include "lodge_json.h"
#include "lodge_serialize_json.h"

#include "str.h"
#include "membuf.h"

#include <inttypes.h>

#define LODGE_SCENE_VERSION 0

static void lodge_scene_entity_to_json(lodge_scene_t scene, const lodge_entity_t entity, lodge_json_t dst_object)
{
	lodge_json_object_set_string(dst_object, strview_static("name"), lodge_scene_get_entity_name(scene, entity));

	uintptr_t entity_parent_id = (uintptr_t)lodge_scene_get_entity_parent(scene, entity);
	if(entity_parent_id) {
		lodge_json_object_set_number(dst_object, strview_static("parent"), (double)entity_parent_id);
	}

	lodge_json_t components_object = lodge_json_object_set_new_object(dst_object, strview_static("components"));
	lodge_scene_entity_components_foreach(scene, entity, component) {
		strview_t component_name = lodge_component_type_get_name(component.type);
		struct lodge_properties *component_properties = lodge_component_type_get_properties(component.type);
		lodge_json_t component_object = lodge_properties_to_json(component_properties, component.value);
		lodge_json_object_set(components_object, component_name, component_object);
	}
}

static lodge_entity_t lodge_scene_entity_from_json(lodge_scene_t scene, uint64_t entity_id, lodge_json_t entity_object)
{
	ASSERT_OR(scene && entity_object) {
		return NULL;
	}

	strview_t entity_name;
	ASSERT_OR(lodge_json_object_get_string(entity_object, strview_static("name"), &entity_name)) {
		return NULL;
	}

	printf("Loading entity: id=%" PRIu64 " name=" STRVIEW_PRINTF_FMT "\n", entity_id, STRVIEW_PRINTF_ARG(entity_name));

	lodge_json_t components_object = lodge_json_object_get_child(entity_object, strview_static("components"));
	ASSERT_OR(components_object) {
		return NULL;
	}

	struct lodge_entity_components_desc components_desc = { 0 };
	for(size_t component_idx = 0, components_count = lodge_json_object_get_child_count(components_object); component_idx < components_count; component_idx++) {
		lodge_json_t component_object = lodge_json_object_get_child_index(components_object, component_idx);

		strview_t component_name;
		ASSERT_OR(lodge_json_object_get_name(component_object, &component_name)) {
			return NULL;
		}
		lodge_component_type_t component_type = lodge_component_types_find(component_name);
		ASSERT_OR(component_type) {
			return NULL;
		}

		membuf_append(membuf_wrap(components_desc.elements), &components_desc.count, &component_type, sizeof(lodge_component_type_t));
	}

	struct lodge_entity_desc entity_desc = {
		.id = entity_id,
		.name = { 0 },
		.parent = NULL, // FIXME(TS): support serializing parent
	};
	strbuf_set(strbuf_wrap(entity_desc.name), entity_name);
	lodge_entity_t entity = lodge_scene_add_entity_from_desc(scene, &entity_desc, &components_desc);
	ASSERT_OR(entity) {
		return NULL;
	}

	//
	// Component properties
	//
	for(size_t component_idx = 0, components_count = lodge_json_object_get_child_count(components_object); component_idx < components_count; component_idx++) {
		lodge_component_type_t component_type = components_desc.elements[component_idx];

		lodge_json_t component_object = lodge_json_object_get_child_index(components_object, component_idx);

		struct lodge_properties *component_properties = lodge_component_type_get_properties(component_type);
		ASSERT_OR(component_properties) {
			goto fail;
		}

		void *component_data = lodge_scene_get_entity_component(scene, entity, component_type);
		ASSERT_OR(component_data) {
			goto fail;
		}

		ASSERT_OR(lodge_properties_from_json(component_object, component_properties, component_data)) {
			goto fail;
		}
	}

	return entity;

fail:
	lodge_scene_remove_entity(scene, entity);
	return NULL;
}

char* lodge_scene_entity_to_text(lodge_scene_t scene, const lodge_entity_t entity, size_t *size_out)
{
	lodge_json_t entity_object = lodge_json_new_object();
	lodge_scene_entity_to_json(scene, entity, entity_object);

	char* json_text = lodge_json_to_string(entity_object);
	if(!json_text) {
		goto fail;
	}
	*size_out = strlen(json_text) + 1;

	lodge_json_free(entity_object);

	return json_text;

fail:
	ASSERT_FAIL("Failed to serialize graph");
	lodge_json_free(entity_object);
	return NULL;
}

lodge_entity_t lodge_scene_entity_from_text(lodge_scene_t scene, strview_t text)
{
	lodge_json_t root = lodge_json_from_string(text);
	ASSERT_OR(root) {
		goto fail;
	}

	lodge_entity_t entity = lodge_scene_entity_from_json(scene, 0, root);
	ASSERT_OR(entity) {
		goto fail;
	}

	lodge_json_free(root);

	return entity;

fail:
	if(root) {
		lodge_json_free(root);
	}
	return NULL;
}

char* lodge_scene_to_text(lodge_scene_t scene, size_t *size_out)
{
	ASSERT_OR(scene) {
		return NULL;
	}

	lodge_json_t root = lodge_json_new_object();
	
	// Version
	lodge_json_object_set_number(root, strview_static("version"), (double)LODGE_SCENE_VERSION);

	// Systems
	{
		lodge_json_t root_systems = lodge_json_object_set_new_object(root, strview_static("systems"));

		lodge_scene_systems_foreach(scene, system) {
			struct lodge_properties *system_properties = lodge_system_type_get_properties(system.type);
			strview_t system_name = lodge_system_type_get_name(system.type);
			lodge_json_t system_object = lodge_properties_to_json(system_properties, system.value);
			lodge_json_object_set(root_systems, system_name, system_object);
		}
	}

	// Entities
	{
		lodge_json_t root_entities = lodge_json_object_set_new_object(root, strview_static("entities"));

		lodge_scene_entities_foreach(scene, entity) {
			char entity_id[256];
			strbuf_setf(strbuf_wrap(entity_id), "%" PRIxPTR, (uintptr_t)entity);
			lodge_json_t entity_object = lodge_json_object_set_new_object(root_entities, strview_wrap(entity_id));
			lodge_scene_entity_to_json(scene, entity, entity_object);
		}
	}

	char* json_text = lodge_json_to_string(root);
	if(!json_text) {
		goto fail;
	}
	*size_out = strlen(json_text) + 1;

	lodge_json_free(root);

	return json_text;

fail:
	ASSERT_FAIL("Failed to serialize graph");
	lodge_json_free(root);
	return NULL;
}

static void* lodge_scene_get_or_add_system_data(lodge_scene_t scene, lodge_system_type_t system_type)
{
	void *system_data = lodge_scene_get_system(scene, system_type);
	if(!system_data) {
		/*lodge_system_t system =*/ lodge_scene_add_system(scene, system_type);
		system_data = lodge_scene_get_system(scene, system_type);
	}
	return system_data;
}

lodge_scene_t lodge_scene_from_text(strview_t text)
{
	struct lodge_scene *scene = calloc(1, lodge_scene_sizeof());
	if(lodge_scene_from_text_inplace(scene, text)) {
		return scene;
	} else {
		free(scene);
		return NULL;
	}
}

bool lodge_scene_from_text_inplace(struct lodge_scene *scene, strview_t text)
{
	lodge_json_t root = lodge_json_from_string(text);
	ASSERT_OR(root) {
		goto fail;
	}

	//
	// Version check
	//
	{
		double version = -1.0;
		ASSERT_OR(lodge_json_object_get_number(root, strview_static("version"), &version)) {
			goto fail;
		}
		ASSERT((int)version == LODGE_SCENE_VERSION);
	}

	lodge_json_t root_systems = lodge_json_object_get_child(root, strview_static("systems"));
	ASSERT_OR(root_systems) {
		goto fail;
	}

	lodge_json_t root_entities = lodge_json_object_get_child(root, strview_static("entities"));
	ASSERT_OR(root_entities) {
		goto fail;
	}

	lodge_scene_new_inplace(scene);

	//
	// Allocate systems
	//
	{
		for(size_t i = 0, count = lodge_json_object_get_child_count(root_systems); i < count; i++) {
			lodge_json_t system_object = lodge_json_object_get_child_index(root_systems, i);

			strview_t system_name;
			ASSERT_OR(lodge_json_object_get_name(system_object, &system_name)) {
				continue;
			}

			lodge_system_type_t system_type = lodge_system_type_find(system_name);
			ASSERT_OR(system_type) {
				continue;
			}

			//
			// FIXME(TS): allocate system in default state
			//
			void *system_data = lodge_scene_get_or_add_system_data(scene, system_type);
			ASSERT_OR(lodge_properties_from_json(system_object, lodge_system_type_get_properties(system_type), system_data)) {
				continue;
			}
		}
	}

	//
	// Allocate entities
	//
	{
		for(size_t entity_idx = 0, entities_count = lodge_json_object_get_child_count(root_entities); entity_idx < entities_count; entity_idx++) {
			lodge_json_t entity_object = lodge_json_object_get_child_index(root_entities, entity_idx);
			
			strview_t entity_object_name;
			ASSERT_OR(lodge_json_object_get_name(entity_object, &entity_object_name)) {
				return false;
			}

			uint64_t entity_id = 0;
			ASSERT_OR(strview_to_u64(entity_object_name, &entity_id)) {
				return false;
			}
			
			lodge_entity_t entity = lodge_scene_entity_from_json(scene, entity_id, lodge_json_object_get_child_index(entity_object, 0));
			ASSERT_OR(entity) { goto fail; }
		}
	}

	ASSERT_NOT_IMPLEMENTED();
	
	return true;

fail:
	lodge_scene_free_inplace(scene);
	lodge_json_free(root);
	return false;
}
