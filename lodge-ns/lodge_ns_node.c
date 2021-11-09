#include "lodge_ns_node.h"

#include "lodge_ns_node_type.h"
#include "lodge_ns_graph.h"
#include "lodge_assert.h"

#include <stdio.h>
#include <string.h>

//
// TODO(TS):
//
//		* Output/input configuration is only safe before the graph has been
//		  configured, the API should reflect this.
//
//		* Pins should probably live in graph, since many nodes only use 1-2 pins.
//		  set_{inputs,outputs} should request pin memory from the graph which guarantees
//		  contiguous memory. This makes graph owner of the total pin budget.
//

static struct lodge_output_pin* lodge_node_input_to_output(lodge_node_t node, lodge_pin_idx_t input_pin_index)
{
    if(input_pin_index >= node->inputs.count) {
		ASSERT_FAIL("Invalid pin index");
		return NULL;
	}

	struct lodge_input_pin *input_pin = &node->inputs.pins[input_pin_index];

	if(!input_pin->connection.node) {
		ASSERT_FAIL("Not connected");
		return NULL;
	}

	struct lodge_output_pin *output_pin = &input_pin->connection.node->outputs.pins[input_pin->connection.pin_index];

	if(output_pin->config.type != input_pin->config.type) {
		ASSERT_FAIL("Invalid in/out pin types");
		return NULL;
	}

	lodge_node_update(input_pin->connection.node);

	return output_pin;
}

void lodge_node_reset(lodge_node_t node)
{
	lodge_variant_reset(&node->config);
	for(lodge_pin_idx_t i = 0; i < node->outputs.count; i++) {
		lodge_variant_reset(&node->outputs.pins[i].data);
	}

	node->inputs = (struct lodge_input_pins) { 0 };
	node->outputs = (struct lodge_output_pins) { 0 };
}


void lodge_node_set_config(lodge_node_t node, struct lodge_variant *config)
{
	lodge_variant_copy(&node->config, config);
	lodge_node_type_configure_node(node->type, node);
}

struct lodge_variant* lodge_node_get_config(lodge_node_t node)
{
	return &node->config;
}

void lodge_node_set_pins(lodge_node_t node, struct lodge_pins *inputs, struct lodge_pins *outputs)
{
	ASSERT_OR(inputs || outputs) { return; }
	if(inputs) {
		lodge_node_set_inputs(node, inputs->count, inputs->pins);
	}
	if(outputs) {
		lodge_node_set_outputs(node, outputs->count, outputs->pins);
	}
}

void lodge_node_set_outputs(lodge_node_t node, const lodge_pin_idx_t count, struct lodge_pin pins[])
{
	ASSERT(!lodge_graph_is_configured(node->graph));
	node->outputs.count = count;
	for(lodge_pin_idx_t i = 0; i < count; i++) {
		node->outputs.pins[i].config = pins[i];
	}
}

void lodge_node_set_inputs(lodge_node_t node, const lodge_pin_idx_t count, struct lodge_pin types[])
{
	ASSERT(!lodge_graph_is_configured(node->graph));
	node->inputs.count = count;
	for(lodge_pin_idx_t i = 0; i < count; i++) {
		node->inputs.pins[i].config = types[i];
	}
}

bool lodge_node_can_connect(lodge_node_t from, lodge_pin_idx_t from_pin, lodge_node_t to, lodge_pin_idx_t to_pin)
{
	if(from->graph != to->graph) {
		return false;
	}

	if(from == to) {
		return false;
	}

	if(to->inputs.pins[to_pin].config.type != from->outputs.pins[from_pin].config.type) {
		return false;
	}

	// TODO(TS): check for closed loops!

	return true;
}

bool lodge_node_connect(lodge_node_t from, lodge_pin_idx_t from_pin, lodge_node_t to, lodge_pin_idx_t to_pin)
{
	ASSERT(!lodge_graph_is_configured(from->graph));

	if(!lodge_node_can_connect(from, from_pin, to, to_pin)) {
		return false;
	}

	to->inputs.pins[to_pin].connection = (struct lodge_pin_connection) {
		.node = from,
		.pin_index = from_pin,
	};
	return true;
}

bool lodge_node_input_is_connected(lodge_node_t node, lodge_pin_idx_t input_pin)
{
	if(input_pin >= node->inputs.count) {
		return false;
	}

	return node->inputs.pins[input_pin].connection.node != NULL;
}


bool lodge_node_input_disconnect(lodge_node_t node, lodge_pin_idx_t input_pin)
{
	ASSERT(!lodge_graph_is_configured(node->graph));

	if(input_pin >= node->inputs.count) {
		return false;
	}

	node->inputs.pins[input_pin].connection.node = NULL;
	node->inputs.pins[input_pin].connection.pin_index = 0;
	return true;
}

bool lodge_node_update(lodge_node_t node)
{
	return lodge_node_type_update_node(node->type, node);
}

void* lodge_node_get_graph_context(lodge_node_t node)
{
	return lodge_graph_get_context(node->graph);
}

void lodge_node_set_value(lodge_node_t node, lodge_pin_idx_t pin_index, const struct lodge_variant *value)
{
	ASSERT(pin_index < node->outputs.count);
	struct lodge_output_pin *pin = &node->outputs.pins[pin_index];
	ASSERT(pin->config.type == value->type);
	lodge_variant_copy(&pin->data, value);
}

const struct lodge_variant* lodge_node_get_value(lodge_node_t node, lodge_pin_idx_t pin_index)
{
	struct lodge_output_pin* output_pin = lodge_node_input_to_output(node, pin_index);
	if(!output_pin) {
		return NULL;
	}
	return &output_pin->data;
}

bool lodge_node_set_value_type(lodge_node_t node, lodge_pin_idx_t pin_index, lodge_type_t type, const void *src)
{
	ASSERT_OR(pin_index < node->outputs.count) { return false; }
	struct lodge_output_pin *pin = &node->outputs.pins[pin_index];
	ASSERT_OR(pin->config.type == type) { return false; }
	lodge_variant_set_type(&pin->data, type, src);
	return true;
}

const void* lodge_node_get_value_type(lodge_node_t node, lodge_pin_idx_t pin_index, lodge_type_t type)
{
	return lodge_variant_get_type(lodge_node_get_value(node, pin_index), type);
}

#define lodge_node_get_set_impl( UNION_FIELD, C_TYPE ) \
	const C_TYPE * lodge_node_get_ ## UNION_FIELD(lodge_node_t node, lodge_pin_idx_t pin_index) \
	{ \
		return lodge_variant_get_ ## UNION_FIELD(lodge_node_get_value(node, pin_index)); \
	} \
	\
	void lodge_node_set_ ## UNION_FIELD(lodge_node_t node, lodge_pin_idx_t pin_index, C_TYPE value) \
	{ \
		struct lodge_variant tmp = lodge_variant_make_ ## UNION_FIELD( value ); \
		lodge_node_set_value(node, pin_index, &tmp); \
	}

lodge_node_get_set_impl(boolean, bool);
lodge_node_get_set_impl(u32, uint32_t);
lodge_node_get_set_impl(u64, uint64_t);
lodge_node_get_set_impl(i32, int32_t);
lodge_node_get_set_impl(i64, int64_t);
lodge_node_get_set_impl(f32, float);
lodge_node_get_set_impl(f64, double);
lodge_node_get_set_impl(vec2, vec2);
lodge_node_get_set_impl(vec3, vec3);
lodge_node_get_set_impl(vec4, vec4);
lodge_node_get_set_impl(mat4, mat4);

#undef lodge_node_get_set_impl