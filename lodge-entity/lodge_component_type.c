#include "lodge_component_type.h"

#include "math4.h"
#include "geometry.h"
#include "membuf.h"

struct lodge_component_type_impl
{
	struct lodge_component_desc		desc;
	lodge_type_t					type;
};

struct lodge_component_type_impl	lodge_component_types[255];
size_t								lodge_component_types_count = 0;

static lodge_component_type_t lodge_component_type_from_index(size_t index)
{
	// Reserve 0 for "invalid" type (NULL)
	return (lodge_component_type_t)((uintptr_t)index + 1);
}

static size_t lodge_component_type_to_index(lodge_component_type_t type)
{
	if(!type) {
		ASSERT_FAIL("NULL handle");
		return 0;
	}
	return (size_t)((uintptr_t)type - 1);
}

static struct lodge_component_desc* lodge_component_type_to_desc(lodge_component_type_t type)
{
	const size_t index = lodge_component_type_to_index(type);
	ASSERT(index < lodge_component_types_count);
	return &lodge_component_types[index].desc;
}

static struct lodge_component_type_impl* lodge_component_type_to_impl(lodge_component_type_t type)
{
	const size_t index = lodge_component_type_to_index(type);
	ASSERT(index < lodge_component_types_count);
	return &lodge_component_types[index];
}

lodge_component_type_t lodge_component_types_get_index(size_t index)
{
	ASSERT(index < lodge_component_types_count);
	if(index < lodge_component_types_count) {
		return lodge_component_type_from_index(index);
	}
	return NULL;
}

size_t lodge_component_types_get_count()
{
	return lodge_component_types_count;
}

lodge_component_type_t lodge_component_types_find(strview_t name)
{
	for(size_t i = 0; i < lodge_component_types_count; i++) {
		if(strview_equals(lodge_component_types[i].desc.name, name)) {
			return lodge_component_type_from_index(i);
		}
	}
	return NULL;
}

lodge_component_type_t lodge_component_type_register(struct lodge_component_desc desc)
{
	membuf_append(
		membuf_wrap(lodge_component_types),
		&lodge_component_types_count,
		&(struct lodge_component_type_impl) {
			.desc = desc,
			.type = lodge_type_register(desc.name, desc.size),
		},
		sizeof(struct lodge_component_type_impl)
	);
	return lodge_component_type_from_index(lodge_component_types_count - 1);
}

lodge_component_type_t lodge_component_find_by_name(strview_t name)
{
	for(size_t i = 0; i < lodge_component_types_count; i++) {
		if(strview_equals(name, lodge_component_types[i].desc.name)) {
			return lodge_component_type_from_index(i);
		}
	}
	return NULL;
}

strview_t lodge_component_type_get_name(lodge_component_type_t type)
{
	struct lodge_component_desc *desc = lodge_component_type_to_desc(type);
	ASSERT(desc);
	return desc ? desc->name : strview_static("");
}

strview_t lodge_component_type_get_description(lodge_component_type_t type)
{
	struct lodge_component_desc *desc = lodge_component_type_to_desc(type);
	ASSERT(desc);
	return desc ? desc->description : strview_static("");
}

size_t lodge_component_type_get_size(lodge_component_type_t type)
{
	struct lodge_component_desc *desc = lodge_component_type_to_desc(type);
	ASSERT(desc);
	return desc ? desc->size : 0;
}

struct lodge_properties* lodge_component_type_get_properties(lodge_component_type_t type)
{
	struct lodge_component_desc *desc = lodge_component_type_to_desc(type);
	ASSERT(desc);
	return desc ? &desc->properties : NULL;
}

void lodge_component_type_new_inplace(lodge_component_type_t component_type, void *dst)
{
	struct lodge_component_desc *desc = lodge_component_type_to_desc(component_type);
	ASSERT(desc);
	if(desc && desc->new_inplace) {
		desc->new_inplace(dst);
	}
}

void lodge_component_type_free_inplace(lodge_component_type_t component_type, void *dst)
{
	struct lodge_component_desc *desc = lodge_component_type_to_desc(component_type);
	ASSERT(desc);
	if(desc && desc->free_inplace) {
		desc->free_inplace(dst);
	}
}

lodge_type_t lodge_component_type_get_type(lodge_component_type_t component_type)
{
	const struct lodge_component_type_impl *impl = lodge_component_type_to_impl(component_type);
	ASSERT(impl);
	if(impl) {
		return impl->type;
	}
	return NULL;
}