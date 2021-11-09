#ifndef _LODGE_NS_NODE_TYPES_HELPERS_H
#define _LODGE_NS_NODE_TYPES_HELPERS_H

//
// Generates boilerplate for a const node holding a specific type.
//
#define lodge_ns_node_const_impl(NS_TYPE, UNION_FIELD, C_TYPE) \
	static bool node_const_ ## UNION_FIELD ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_outputs(node, 1, (struct lodge_pin[]) { \
			{ \
				.name = strview("out"), \
				.type = NS_TYPE, \
			} \
		}); \
		if(!lodge_variant_is_set(&node->config)) { \
			lodge_variant_set_ ## UNION_FIELD (&node->config, (C_TYPE) { 0 }); \
		} \
		const C_TYPE *config = lodge_variant_get_ ## UNION_FIELD (&node->config); \
		if(config) { \
			lodge_node_set_value(node, 0, &node->config); \
		} \
		return true; \
	}

//
// Must be called once per type, before any `lodge_ns_node_unary_func_impl`, to set up helpers.
//
#define lodge_ns_node_unary_func_decl(TYPE, C_TYPE, LODGE_TYPE) \
	static bool node_unary_ ## TYPE ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_pins(node, \
			&(struct lodge_pins) { \
				.count = 1,\
				.pins = { \
					{ \
						.name = strview("in"), \
						.type = LODGE_TYPE, \
					}, \
				}, \
			}, \
			&(struct lodge_pins) { \
				.count = 1, \
				.pins = { \
					{ \
						.name = strview("out"), \
						.type = LODGE_TYPE \
					}, \
				} \
			} \
		); \
		return true; \
	} \
	\
	typedef C_TYPE (*node_unary_ ## TYPE ## _run_func_t)(const C_TYPE lhs); \
	\
	static bool node_unary_ ## TYPE ## _run(struct lodge_node *node, node_unary_ ## TYPE ## _run_func_t func) \
	{ \
		const C_TYPE *in = lodge_node_get_ ## TYPE (node, 0); \
		if(!in) { \
			return false; \
		} \
		lodge_node_set_ ## TYPE (node, 0, func(*in)); \
		return true; \
	}

//
// Generates boilerplate for a node run function:
//
//		`<C_TYPE> node_unary_<TYPE>_<FUNC_NAME>_run(<C_TYPE> in)`
//
// that can be used with `lodge_node_type_register`.
//
#define lodge_ns_node_unary_func_impl(FUNC_NAME, TYPE, C_TYPE) \
	static C_TYPE node_unary_ ## TYPE ## _ ## FUNC_NAME ## _run_impl(const C_TYPE in); \
	\
	static bool node_unary_ ## TYPE ## _ ## FUNC_NAME ## _run(struct lodge_node *node) \
	{ \
		return node_unary_ ## TYPE ## _run(node, &node_unary_ ## TYPE ## _ ## FUNC_NAME ## _run_impl); \
	} \
	\
	static C_TYPE node_unary_ ## TYPE ## _ ## FUNC_NAME ## _run_impl(const C_TYPE in) \

//
// Must be called once per type, before any `lodge_ns_node_binary_func_impl`, to set up helpers.
//
#define lodge_ns_node_binary_func_decl( TYPE, C_TYPE, LODGE_TYPE ) \
	static bool node_binary_ ## TYPE ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_pins(node, \
			&(struct lodge_pins) { \
				.count = 2, \
				.pins = { \
					{ \
						.name = strview("lhs"), \
						.type = LODGE_TYPE, \
					}, \
					{ \
						.name = strview("rhs"), \
						.type = LODGE_TYPE, \
					} \
				}, \
			}, \
			&(struct lodge_pins) { \
				.count = 1, \
				.pins = { \
					{ \
						.name = strview("out"), \
						.type = LODGE_TYPE \
					} \
				}, \
			} \
		); \
		return true; \
	} \
	\
	typedef C_TYPE (*node_binary_ ## TYPE ## _run_func_t)(const C_TYPE lhs, const C_TYPE rhs); \
	\
	static bool node_binary_ ## TYPE ## _run(struct lodge_node *node, node_binary_ ## TYPE ## _run_func_t func) \
	{ \
		const C_TYPE *lhs = lodge_node_get_ ## TYPE (node, 0); \
		if(!lhs) { \
			return false; \
		} \
		const C_TYPE *rhs = lodge_node_get_ ## TYPE (node, 1); \
		if(!rhs) { \
			return false; \
		} \
		\
		lodge_node_set_ ## TYPE (node, 0, func(*lhs, *rhs)); \
		return true; \
	}


//
// Must be called once per type, before any `lodge_ns_node_binary_func_impl`, to set up helpers.
//
#define lodge_ns_node_binary_func_decl( TYPE, C_TYPE, LODGE_TYPE ) \
	static bool node_binary_ ## TYPE ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_pins(node, \
			&(struct lodge_pins) { \
				.count = 2, \
				.pins = { \
					{ \
						.name = strview("lhs"), \
						.type = LODGE_TYPE, \
					}, \
					{ \
						.name = strview("rhs"), \
						.type = LODGE_TYPE, \
					} \
				}, \
			}, \
			&(struct lodge_pins) { \
				.count = 1, \
				.pins = { \
					{ \
						.name = strview("out"), \
						.type = LODGE_TYPE \
					} \
				} \
			} \
		); \
		return true; \
	} \
	\
	typedef C_TYPE (*node_binary_ ## TYPE ## _run_func_t)(const C_TYPE lhs, const C_TYPE rhs); \
	\
	static bool node_binary_ ## TYPE ## _run(struct lodge_node *node, node_binary_ ## TYPE ## _run_func_t func) \
	{ \
		const C_TYPE *lhs = lodge_node_get_ ## TYPE (node, 0); \
		if(!lhs) { \
			return false; \
		} \
		const C_TYPE *rhs = lodge_node_get_ ## TYPE (node, 1); \
		if(!rhs) { \
			return false; \
		} \
		\
		lodge_node_set_ ## TYPE (node, 0, func(*lhs, *rhs)); \
		return true; \
	}

//
// Generates boilerplate for a node run function:
//
//		`<C_TYPE> node_binary_<TYPE>_<FUNC_NAME>_run(<C_TYPE> lhs, <C_TYPE> rhs)`
//
// that can be used with `lodge_node_type_register`.
//
#define lodge_ns_node_binary_func_impl(FUNC_NAME, TYPE, C_TYPE) \
	static C_TYPE node_binary_ ## TYPE ## _ ## FUNC_NAME ## _run_impl(const C_TYPE lhs, const C_TYPE rhs); \
	\
	static bool node_binary_ ## TYPE ## _ ## FUNC_NAME ## _run(struct lodge_node *node) \
	{ \
		return node_binary_ ## TYPE ## _run(node, &node_binary_ ## TYPE ## _ ## FUNC_NAME ## _run_impl); \
	} \
	\
	static C_TYPE node_binary_ ## TYPE ## _ ## FUNC_NAME ## _run_impl(const C_TYPE lhs, const C_TYPE rhs) \


#if 0
#define lodge_ns_node_math_ternary_func_impl() \
	static bool node_math_ ## FUNC ## _ ## UNION_FIELD ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_pins(node, \
			1, (struct lodge_pin[]) { \
				{ \
					.name = strview("in"), \
					.type = NS_TYPE ## , \
				}, \
			}, \
			1, (struct lodge_pin[]) { \
				{ \
					.name = strview("out"), \
					.type = NS_TYPE ## , \
				} \
			} \
		); \
		return true; \
	} \
	\
	static bool node_math_ ## FUNC ## _ ## UNION_FIELD ## _run(struct lodge_node *node) \
	{ \
		const C_TYPE *t = lodge_variant_get_ ## UNION_FIELD ## (lodge_node_get_value(node, 0)); \
		if(!t) { \
			return false; \
		} \
		lodge_node_set_ ## UNION_FIELD ## (node, 0, (C_TYPE) FUNC ## (*t)); \
		return true; \
	}
#endif

#endif