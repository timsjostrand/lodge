#ifndef _LODGE_NS_NODE_H
#define _LODGE_NS_NODE_H

#include "lodge_ns.h"
#include "lodge_type.h"
#include "lodge_variant.h"
#include "math4.h"
#include "strview.h"

struct lodge_pin
{
	lodge_type_t				type;
	strview_t					name;
};

struct lodge_output_pin
{
	struct lodge_pin			config;
	struct lodge_variant		data;
};

struct lodge_output_pins
{
	struct lodge_output_pin		pins[8];
	lodge_pin_idx_t				count;
};

struct lodge_pin_connection
{
	lodge_node_t				node;
	lodge_pin_idx_t				pin_index;
};

struct lodge_input_pin
{
	struct lodge_pin			config;
	//bool						optional; // flags could go here
	struct lodge_pin_connection	connection;
};

struct lodge_input_pins
{
	struct lodge_input_pin		pins[8];
	lodge_pin_idx_t				count;
};

struct lodge_node
{
	lodge_node_id_t				id;
	lodge_node_type_t			type;

	lodge_graph_t				graph;

	struct lodge_input_pins		inputs;
	struct lodge_output_pins	outputs;

	struct lodge_variant		config;
};

struct lodge_pins
{
	lodge_pin_idx_t				count;

	// "pins" and "elements" are aliased.
	union
	{
		struct lodge_pin		pins[];
		struct lodge_pin		elements[];
	};
};

void							lodge_node_reset(lodge_node_t node);

void							lodge_node_set_config(lodge_node_t node, struct lodge_variant *config);
struct lodge_variant*			lodge_node_get_config(lodge_node_t node);

void							lodge_node_set_pins(lodge_node_t node, struct lodge_pins *inputs, struct lodge_pins *outputs);
void							lodge_node_set_inputs(lodge_node_t node, const lodge_pin_idx_t count, struct lodge_pin pins[]);
void							lodge_node_set_outputs(lodge_node_t node, const lodge_pin_idx_t count, struct lodge_pin pins[]);

//
// TODO(TS): overloads with pin names instead of indices
//
bool							lodge_node_can_connect(lodge_node_t from, lodge_pin_idx_t from_pin, lodge_node_t to, lodge_pin_idx_t to_pin);
bool							lodge_node_connect(lodge_node_t from, lodge_pin_idx_t from_pin, lodge_node_t to, lodge_pin_idx_t to_pin);

bool							lodge_node_input_is_connected(lodge_node_t node, lodge_pin_idx_t input_pin);
bool							lodge_node_input_disconnect(lodge_node_t node, lodge_pin_idx_t input_pin);

bool							lodge_node_update(lodge_node_t node);

void*							lodge_node_get_graph_context(lodge_node_t node);

void							lodge_node_set_value(lodge_node_t node, lodge_pin_idx_t pin_index, const struct lodge_variant *value);
const struct lodge_variant*		lodge_node_get_value(lodge_node_t node, lodge_pin_idx_t pin_index);

bool							lodge_node_set_value_type(lodge_node_t node, lodge_pin_idx_t pin_index, lodge_type_t type, const void *src);
const void*						lodge_node_get_value_type(lodge_node_t node, lodge_pin_idx_t pin_index, lodge_type_t type);


#define lodge_node_get_set_decl( TYPE_FUNC_NAME, C_TYPE ) \
	const C_TYPE *	lodge_node_get_ ## TYPE_FUNC_NAME(lodge_node_t node, lodge_pin_idx_t pin_index); \
	void			lodge_node_set_ ## TYPE_FUNC_NAME(lodge_node_t node, lodge_pin_idx_t pin_index, C_TYPE value)

lodge_node_get_set_decl(boolean, bool);
lodge_node_get_set_decl(u32, uint32_t);
lodge_node_get_set_decl(u64, uint64_t);
lodge_node_get_set_decl(i32, int32_t);
lodge_node_get_set_decl(i64, int64_t);
lodge_node_get_set_decl(f32, float);
lodge_node_get_set_decl(f64, double);
lodge_node_get_set_decl(vec2, vec2);
lodge_node_get_set_decl(vec3, vec3);
lodge_node_get_set_decl(vec4, vec4);
lodge_node_get_set_decl(mat4, mat4);

#undef lodge_node_get_set_decl

#endif