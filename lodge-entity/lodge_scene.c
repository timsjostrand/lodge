#include "lodge_scene.h"

#include "lodge_entity_type.h"
#include "lodge_component_type.h"
#include "lodge_system_type.h"
#include "lodge_bound_func.h"

#include "str.h"
#include "membuf.h"
#include "strbuf.h"
#include "sparse_set.h"

#include <stdbool.h>

struct lodge_system
{
	lodge_system_type_t				type;
	void							*data;
};

struct lodge_component_set
{
	lodge_component_type_t			type;
	sparse_set_t					set;
};

struct lodge_scene
{
	float							time;

	sparse_set_t					entities;
	size_t							entities_last_id;

	struct lodge_component_set		component_sets[32];
	size_t							component_sets_count;

	struct lodge_system				systems[32];
	size_t							systems_count;

	struct lodge_scene_funcs		funcs;
};

#define COMPONENTS_COUNT_DEFAULT	(256)
#define ENTITIES_COUNT_DEFAULT		(256)
#define SPARSE_INDICES_PER_PAGE		(4 * 1024)

static struct lodge_component_set* lodge_component_set_get_by_type(lodge_scene_t scene, lodge_component_type_t type)
{
	for(size_t i = 0, count = scene->component_sets_count; i < count; i++) {
		struct lodge_component_set *component_set = &scene->component_sets[i];
		if(component_set->type == type) {
			return component_set;
		}
	}
	return NULL;
}

static struct lodge_component_set* lodge_component_set_get_or_add_by_type(lodge_scene_t scene, lodge_component_type_t type)
{
	struct lodge_component_set *component_set = lodge_component_set_get_by_type(scene, type);

	if(!component_set) {
		component_set = membuf_append(
			membuf_wrap(scene->component_sets),
			&scene->component_sets_count,
			&(struct lodge_component_set) {
				.type = type,
				.set = sparse_set_new(lodge_component_type_get_size(type), COMPONENTS_COUNT_DEFAULT, SPARSE_INDICES_PER_PAGE),
			},
			sizeof(struct lodge_component_set)
		);
	}

	ASSERT(component_set);

	return component_set;
}

static lodge_component_t lodge_scene_add_component_internal(lodge_scene_t scene, size_t entity_id, lodge_component_type_t type)
{
	struct lodge_component_set *component_set = lodge_component_set_get_or_add_by_type(scene, type);
	ASSERT(component_set);
	if(!component_set) {
		return NULL;
	}

	void *component = sparse_set_set_no_init(component_set->set, entity_id);
	lodge_component_type_new_inplace(type, component);

	return component;
}

static struct lodge_entity_desc* lodge_entity_desc_from_entity(lodge_scene_t scene, lodge_entity_t entity)
{
	if(!entity) {
		return NULL;
	}
	const size_t entity_id = (size_t)((uintptr_t)entity - 1);
	return sparse_set_get(scene->entities, entity_id);
}

static lodge_entity_t lodge_entity_desc_to_entity(struct lodge_entity_desc *entity)
{
	return entity ? (lodge_entity_t)((uintptr_t)entity->id + 1) : NULL;
}

void lodge_scene_new_inplace(lodge_scene_t scene)
{
	scene->time = 0.0f;
	//scene->entities_count = 0;
	scene->entities_last_id = 0;
	scene->systems_count = 0;

	scene->entities = sparse_set_new(sizeof(struct lodge_entity_desc), ENTITIES_COUNT_DEFAULT, SPARSE_INDICES_PER_PAGE);
}

void lodge_scene_free_inplace(lodge_scene_t scene)
{
	//
	// Free systems
	//
	for(size_t i = 0; i < scene->systems_count; i++) {
		lodge_system_type_free_inplace(scene->systems[i].type, scene->systems[i].data, scene);
	}

	//
	// Free components
	//
	for(size_t i = 0, count = scene->component_sets_count; i < count; i++) {
		struct lodge_component_set *component_set = &scene->component_sets[i];
		lodge_component_type_t type = component_set->type;

		for(void *it = sparse_set_it_begin(component_set->set); it; it = sparse_set_it_next(component_set->set, it)) {
			lodge_component_type_free_inplace(type, it);
		}
	}

	//
	// Free entities
	//
	sparse_set_free(scene->entities);
}

size_t lodge_scene_sizeof()
{
	return sizeof(struct lodge_scene);
}

void lodge_scene_update(lodge_scene_t scene, float dt)
{
	scene->time += dt;
	for(size_t i = 0, count = scene->systems_count; i < count; i++) {
		struct lodge_system *system = &scene->systems[i];
		lodge_system_type_update(system->type, system->data, scene, dt);
	}
}

lodge_entity_t lodge_scene_add_entity_from_desc(lodge_scene_t scene, const struct lodge_entity_desc *entity_desc, const struct lodge_entity_components_desc *components_desc)
{
	ASSERT_OR(scene && entity_desc) {
		return NULL;
	}

	size_t entity_id = entity_desc->id;
	if(entity_id == 0) {
		entity_id = scene->entities_last_id + 1;
	}

	ASSERT_OR(entity_id > scene->entities_last_id) {
		return NULL;
	}

	scene->entities_last_id = max(scene->entities_last_id, entity_id);

	struct lodge_entity_desc *tmp = sparse_set_set(scene->entities, entity_id, entity_desc);
	tmp->id = entity_id;

	if(components_desc) {
		for(size_t i = 0, count = components_desc->count; i < count; i++) {
			lodge_component_t component = lodge_scene_add_component_internal(scene, tmp->id, components_desc->elements[i]);
			ASSERT(component);
		}
	}

	return lodge_entity_desc_to_entity(tmp);
}

lodge_entity_t lodge_scene_add_entity_from_type(lodge_scene_t scene, lodge_entity_type_t entity_type)
{
	ASSERT_OR(scene && entity_type) {
		return NULL;
	}

	const struct lodge_entity_components_desc *components_desc = lodge_entity_type_get_components(entity_type);
	ASSERT_OR(components_desc) {
		return NULL;
	}

	struct lodge_entity_desc entity_desc = {
		.id = scene->entities_last_id + 1,
		.name = '\0',
		.parent = NULL,
	};

	strview_t entity_type_name = lodge_entity_type_get_name(entity_type);
	strbuf_setf(strbuf_wrap(entity_desc.name),
		STRVIEW_PRINTF_FMT "_%zu",
		STRVIEW_PRINTF_ARG(entity_type_name),
		entity_desc.id
	);

	return lodge_scene_add_entity_from_desc(scene, &entity_desc, components_desc);
}

lodge_component_t lodge_scene_add_entity_component(lodge_scene_t scene, lodge_entity_t entity, lodge_component_type_t component_type)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);
	if(!entity_desc) {
		return NULL;
	}
	return lodge_scene_add_component_internal(scene, entity_desc->id, component_type);
}

bool lodge_scene_remove_entity(lodge_scene_t scene, lodge_entity_t entity)
{
	ASSERT_NOT_IMPLEMENTED();
	return false;
}

lodge_system_t lodge_scene_add_system(lodge_scene_t scene, lodge_system_type_t system_type)
{
	ASSERT(system_type);
	struct lodge_system *system = membuf_append(
		membuf_wrap(scene->systems),
		&scene->systems_count,
		&(struct lodge_system) {
			.type = system_type,
			.data = NULL,
		},
		sizeof(struct lodge_system)
	);

	system->data = calloc(1, lodge_system_type_sizeof(system_type));
	lodge_system_type_new_inplace(system_type, system->data, scene);

	return system;
}

void* lodge_scene_get_system(lodge_scene_t scene, lodge_system_type_t system_type)
{
	ASSERT(scene);
	ASSERT(system_type);
	for(size_t i = 0, count = scene->systems_count; i<count; i++) {
		if(scene->systems[i].type == system_type) {
			return scene->systems[i].data;
		}
	}
	return NULL;
}

void* lodge_scene_get_entity_component(lodge_scene_t scene, lodge_entity_t entity, lodge_component_type_t type)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);
	ASSERT_OR(entity_desc) {
		return NULL;
	}
	struct lodge_component_set *component_set = lodge_component_set_get_by_type(scene, type);
	if(!component_set) {
		return NULL;
	}
	return sparse_set_get(component_set->set, (uint32_t)entity_desc->id);
}

strview_t lodge_scene_get_entity_name(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);
	return entity_desc ? strview_wrap(entity_desc->name) : strview_static("");
}

lodge_entity_t lodge_scene_get_entity_parent(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);
	return entity_desc ? entity_desc->parent : NULL;
}

lodge_entity_t lodge_scene_get_component_entity(lodge_scene_t scene, lodge_component_type_t type, const void *component)
{
	struct lodge_component_set *component_set = lodge_component_set_get_by_type(scene, type);
	if(!component_set) {
		return NULL;
	}

	const uint32_t index = sparse_set_get_index(component_set->set, component);

	struct lodge_entity_desc *entity_desc = sparse_set_get(scene->entities, index);
	if(!entity_desc) {
		return NULL;
	}

	return lodge_entity_desc_to_entity(entity_desc);
}

//
// TODO(TS): scene should own a cache (or sole owner?) of the entity tree structure,
// so editors et al can easily iterate.
//
void lodge_scene_set_entity_parent(lodge_scene_t scene, lodge_entity_t entity, lodge_entity_t parent)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);
	ASSERT(entity_desc);
	if(entity_desc) {
		entity_desc->parent = parent;
		// TODO(TS): callback?
	}
}

lodge_entity_t lodge_scene_entities_begin(lodge_scene_t scene)
{
	struct lodge_entity_desc *first = sparse_set_it_begin(scene->entities);
	return lodge_entity_desc_to_entity(first);
}

lodge_entity_t lodge_scene_entities_next(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_entity_desc *entity_instance = lodge_entity_desc_from_entity(scene, entity);
	struct lodge_entity_desc *next = sparse_set_it_next(scene->entities, entity_instance);
	return lodge_entity_desc_to_entity(next);
}

void* lodge_scene_components_begin(lodge_scene_t scene, lodge_component_type_t type)
{
	struct lodge_component_set *componet_set = lodge_component_set_get_by_type(scene, type);
	if(!componet_set) {
		return NULL;
	}
	return sparse_set_it_begin(componet_set->set);
}

void* lodge_scene_components_next(lodge_scene_t scene, lodge_component_type_t type, void *component)
{
	struct lodge_component_set *componet_set = lodge_component_set_get_by_type(scene, type);
	if(!componet_set) {
		return NULL;
	}
	return sparse_set_it_next(componet_set->set, component);
}

struct lodge_component_it lodge_scene_entity_components_begin(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);

	for(size_t i = 0, count = scene->component_sets_count; i < count; i++) {
		struct lodge_component_set *component_set = &scene->component_sets[i];

		void *component = sparse_set_get(component_set->set, entity_desc->id);
		if(component) {
			return (struct lodge_component_it) {
				.value = component,
				.type = component_set->type,
				.index = i,
			};
		}
	}

	return (struct lodge_component_it) { .value = NULL, .type = NULL, .index = 0 };
}

struct lodge_component_it lodge_scene_entity_components_next(lodge_scene_t scene, lodge_entity_t entity, struct lodge_component_it previous)
{
	struct lodge_entity_desc *entity_desc = lodge_entity_desc_from_entity(scene, entity);

	bool return_next = false;

	for(size_t i = previous.index + 1, count = scene->component_sets_count; i < count; i++) {
		struct lodge_component_set *component_set = &scene->component_sets[i];

		void *component = sparse_set_get(component_set->set, entity_desc->id);
		if(component) {
			return (struct lodge_component_it) {
				.value = component,
				.type = component_set->type,
				.index = i,
			};
		}
	}

	return (struct lodge_component_it) { .value = NULL, .type = NULL, .index = 0 };
}

struct lodge_system_it lodge_scene_systems_begin(lodge_scene_t scene)
{
	ASSERT(scene);
	return (scene->systems_count > 0)
		? (struct lodge_system_it) { .value = scene->systems[0].data, .type = scene->systems[0].type }
		: (struct lodge_system_it) { .value = NULL, .type = NULL };
}

struct lodge_system_it lodge_scene_systems_next(lodge_scene_t scene, struct lodge_system_it current_it)
{
	ASSERT(scene);
	ASSERT(current_it.value);
	bool return_next = false;
	for(size_t i = 0, count = scene->systems_count; i < count; i++) {
		if(return_next) {
			return (struct lodge_system_it) {
				.value = scene->systems[i].data,
				.type = scene->systems[i].type,
			};
		}
		if(scene->systems[i].data == current_it.value) {
			return_next = true;
		}
	}

	return (struct lodge_system_it) { .value = NULL, .type = NULL };
}

void lodge_scene_render(lodge_scene_t scene, struct lodge_system_render_params *render_params)
{
	for(size_t i = 0, count = scene->systems_count; i < count; i++) {
		struct lodge_system *system = &scene->systems[i];
		lodge_system_type_render(system->type, system->data, scene, render_params);
	}
}

float lodge_scene_get_time(lodge_scene_t scene)
{
	return scene->time;
}

struct lodge_scene_funcs* lodge_scene_get_funcs(lodge_scene_t scene)
{
	return scene ? &scene->funcs : NULL;
}

void lodge_scene_set_entity_selected(lodge_scene_t scene, lodge_entity_t entity, bool selected)
{
	ASSERT_OR(scene && entity) { return; }
	struct lodge_scene_funcs *funcs = lodge_scene_get_funcs(scene);
	ASSERT_OR(funcs) { return; }
	if(lodge_bound_func_is_set(funcs->set_entity_selected)) {
		lodge_bound_func_call(funcs->set_entity_selected, entity, selected);
	}
}

bool lodge_scene_is_entity_selected(lodge_scene_t scene, lodge_entity_t entity)
{
	ASSERT_OR(scene && entity) { return false; }
	struct lodge_scene_funcs *funcs = lodge_scene_get_funcs(scene);
	ASSERT_OR(funcs) { return false; }
	if(!lodge_bound_func_is_set(funcs->set_entity_selected)) { return false; }
	return lodge_bound_func_call(funcs->is_entity_selected, entity);
}
