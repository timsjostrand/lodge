#include "lodge_system_type.h"

#include "membuf.h"

struct lodge_system_type_desc system_descs[256];
size_t system_descs_count = 0;

static struct lodge_system_type_desc* lodge_system_type_desc_from_handle(lodge_system_type_t system)
{
	return (struct lodge_system_type_desc *) system;
}

lodge_system_type_t lodge_system_type_register(struct lodge_system_type_desc desc)
{
	ASSERT(desc.plugin);
	ASSERT(!strview_empty(desc.name));

	for(size_t i = 0, count = desc.properties.count; i < count; i++) {
		struct lodge_property *property = &desc.properties.elements[i];
		ASSERT(property->type);
		ASSERT(property->offset < desc.size);
		ASSERT((property->offset + lodge_type_get_size(property->type)) <= desc.size);
		ASSERT(!strview_empty(property->name));

		for(size_t j = 0, count = desc.properties.count; j < count; j++) {
			if(i == j) {
				continue;;
			}
			struct lodge_property *lhs = &desc.properties.elements[i];
			struct lodge_property *rhs = &desc.properties.elements[j];
			ASSERT(!strview_equals(lhs->name, rhs->name));
			ASSERT(lhs->offset != rhs->offset);
		}
	}

	return (lodge_system_type_t) membuf_append(membuf_wrap(system_descs), &system_descs_count, &desc, sizeof(struct lodge_system_type_desc));
}

void lodge_system_type_new_inplace(lodge_system_type_t system_type, void *dst, lodge_scene_t scene)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	if(system_desc && system_desc->new_inplace) {
		system_desc->new_inplace(dst, scene);
	}
}

void lodge_system_type_free_inplace(lodge_system_type_t system_type, void *dst, lodge_scene_t scene)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	if(system_desc && system_desc->free_inplace) {
		system_desc->free_inplace(dst, scene);
	}
}

size_t lodge_system_type_sizeof(lodge_system_type_t system_type)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	return system_desc ? system_desc->size : 0;
}

void lodge_system_type_update(lodge_system_type_t system_type, void *system, lodge_scene_t scene, float dt)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	if(system_desc && system_desc->update) {
		system_desc->update(system, system_type, scene, dt);
	}
}

void lodge_system_type_render(lodge_system_type_t system_type, void *system, lodge_scene_t scene, struct lodge_system_render_params *render_params)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	if(system_desc && system_desc->render) {
		system_desc->render(system, scene, render_params);
	}
}

strview_t lodge_system_type_get_name(lodge_system_type_t system_type)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	return system_desc ? system_desc->name : strview_static("");
}

struct lodge_properties* lodge_system_type_get_properties(lodge_system_type_t system_type)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	return system_desc ? &system_desc->properties : NULL;
}

void* lodge_system_type_get_plugin(lodge_system_type_t system_type)
{
	struct lodge_system_type_desc *system_desc = lodge_system_type_desc_from_handle(system_type);
	ASSERT(system_desc);
	return system_desc ? system_desc->plugin : NULL;
}

lodge_system_type_t lodge_system_type_find(strview_t name)
{
	for(size_t i = 0; i < system_descs_count; i++) {
		if(strview_equals(system_descs[i].name, name)) {
			return (lodge_system_type_t)&system_descs[i];
		}
	}
	return NULL;
}


