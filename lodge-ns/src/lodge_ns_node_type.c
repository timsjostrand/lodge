#include "lodge_ns_node_type.h"

#include "lodge_variant.h"
#include "lodge_ns_node.h"

#include "membuf.h"
#include "strbuf.h"
#include "lodge_assert.h"

struct lodge_node_type
{
	char								name[255];
	struct lodge_variant				default_config;
	lodge_node_type_configure_func_t	configure;
	lodge_node_type_run_func_t			run;
};

//
// TODO(TS): these should probably live in a "node system"
//
//		* A "node system" owns the types and graphs that can run in that system.
//		* A system could be; particles, entities, etc
//
struct lodge_node_type	lodge_ns_node_types[256];
size_t					lodge_ns_node_types_count = 0;

lodge_node_type_t lodge_node_type_register(strview_t name, lodge_node_type_configure_func_t configure_func, lodge_node_type_run_func_t run_func)
{
	lodge_node_type_t node_type = membuf_append(membuf_wrap(lodge_ns_node_types), &lodge_ns_node_types_count,
		&(struct lodge_node_type) {
			.name = { '\0' },
			.configure = configure_func,
			.run = run_func,
			.default_config = { .type = LODGE_TYPE_NONE },
		},
		sizeof(struct lodge_node_type)
	);

	strbuf_set(strbuf_wrap(node_type->name), name);

	return node_type;
}

lodge_node_type_t lodge_node_type_find(strview_t name)
{
	for(size_t i = 0; i < lodge_ns_node_types_count; i++) {
		struct lodge_node_type *node_type = &lodge_ns_node_types[i];
		if(strview_equals(strbuf_wrap_and(node_type->name, strbuf_to_strview), name)) {
			return node_type;
		}
	}
	return NULL;
}

bool lodge_node_type_update_node(lodge_node_type_t node_type, lodge_node_t node)
{
	if(!node_type->run) {
		return true;
	}
	return node_type->run(node);
}

bool lodge_node_type_configure_node(lodge_node_type_t node_type, lodge_node_t node)
{
	if(lodge_variant_is_set(&node_type->default_config)) {
		lodge_variant_copy(lodge_node_get_config(node), &node_type->default_config);
	}
	// No configuration required
	if(!node_type->configure) {
		return true;
	}
	if(!node_type->configure(node)) {
		return false;
	}

	for(lodge_pin_idx_t input_pin = 0; input_pin < node->inputs.count; input_pin++) {
		//
		// TODO(TS): Add support for explicitly optional pins
		//
		if(!node->inputs.pins[input_pin].connection.node) {
			return false;
		}
	}

	return true;
}

size_t lodge_node_type_get_count()
{
	return lodge_ns_node_types_count;
}

lodge_node_type_t lodge_node_type_get_index(size_t index)
{
	ASSERT(index < lodge_ns_node_types_count);
	return &lodge_ns_node_types[index];
}

strview_t lodge_node_type_get_name(lodge_node_type_t node_type)
{
	return strbuf_wrap_and(node_type->name, strbuf_to_strview);
}

void lodge_node_type_set_default_config(lodge_node_type_t node_type, struct lodge_variant *config)
{
	lodge_variant_copy(&node_type->default_config, config);
}
