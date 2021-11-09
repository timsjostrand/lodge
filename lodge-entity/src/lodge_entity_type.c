#include "lodge_entity_type.h"

#include "membuf.h"
#include "strbuf.h"
#include "str.h"
#include "lodge_platform.h"

struct lodge_entity_type
{
	char									name[255];
	struct lodge_entity_components_desc		components;
};

struct lodge_entity_type					lodge_entity_types[255];
size_t										lodge_entity_types_count;

lodge_entity_type_t lodge_entity_types_get_index(size_t index)
{
	if(index < LODGE_ARRAYSIZE(lodge_entity_types)) {
		return &lodge_entity_types[index];
	}
	return NULL;
}

size_t lodge_entity_types_get_count()
{
	return lodge_entity_types_count;
}

lodge_entity_type_t lodge_entity_type_register(struct lodge_entity_type_desc desc)
{
	lodge_entity_type_t entity_type = membuf_append_no_init(
		membuf_wrap(lodge_entity_types),
		&lodge_entity_types_count
	);

	strbuf_set(strbuf_wrap(entity_type->name), desc.name);
	entity_type->components = desc.components;

	return entity_type;
}

strview_t lodge_entity_type_get_name(lodge_entity_type_t entity_type)
{
	return entity_type ? strview_wrap(entity_type->name) : strview_static("");
}

const struct lodge_entity_components_desc* lodge_entity_type_get_components(lodge_entity_type_t entity_type)
{
	return entity_type ? &entity_type->components : NULL;
}
