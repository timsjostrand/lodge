#include "lodge_properties.h"

#include "membuf.h"

#include <string.h>

const void* lodge_property_get(const struct lodge_property *property, const void *object)
{
	ASSERT(property);
	ASSERT(object);
	return (const void*)(((const char*)object) + property->offset);
}

void lodge_property_set(struct lodge_property *property, void *object, const void *src)
{
	ASSERT(!(property->flags & LODGE_PROPERTY_FLAG_READ_ONLY));

	void *dst = (void*)lodge_property_get(property, object);
	ASSERT(dst);
	if(!dst) {
		return;
	}

	if(property->pre_modified) {
		if(!property->pre_modified(property, object, dst, src)) {
			return;
		}
	}

	//
	// TODO(TS): may want to be able to register custom copy functions through a FUNC_INDEX
	//
	memcpy(dst, src, lodge_type_get_size(property->type));

	if(property->on_modified) {
		property->on_modified(property, object);
	}
}

#if 0
struct lodge_variant lodge_property_clone(struct lodge_property *property, void *object)
{

}
#endif

#if 0
void lodge_object_to_json(const struct lodge_properties *properties, const void *object)
{
	for(size_t i = 0; i < properties->count; i++) {
		const struct lodge_property *property = &properties->elements[i];

		if(property->flags & LODGE_PROPERTY_FLAG_TRANSIENT) {
			continue;
		}

		const void *property_data = lodge_property_get(property, object);

		lodge_type_to_json_func_t to_json = lodge_type_get_func(property->type, LODGE_TYPE_FUNC_INDEX_TO_JSON);
		ASSERT(to_json);

		lodge_json_t json_node = to_json(property_data);
		ASSERT(json_node);
	}
}
#endif

bool lodge_property_meta_equals(struct lodge_property *lhs, struct lodge_property *rhs)
{
	return lhs->type == rhs->type
		&& lhs->offset == rhs->offset
		&& lhs->flags == rhs->flags
		&& lhs->on_modified == rhs->on_modified
		&& strview_equals(lhs->name, rhs->name);
}
