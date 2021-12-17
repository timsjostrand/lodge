#include "lodge_ns_node_types_helpers.h"
#include "lodge_variant.h"
#include "lodge_ns_node_type.h"
#include "lodge_ns_graph.h"
#include "lodge_ns_node.h"

// 
// TODO(TS): this could/should be generalized to take any enumerable type, esp. enums (generate inputs for each enumerable value).
//

#define lodge_node_type_select_decl( TYPE, C_TYPE, LODGE_TYPE ) \
	static bool node_select_ ## TYPE ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_pins(node, \
			&(struct lodge_pins) { \
				.count = 3, \
				.pins = { \
					{ \
						.name = strview("true"), \
						.type = LODGE_TYPE, \
					}, \
					{ \
						.name = strview("false"), \
						.type = LODGE_TYPE, \
					}, \
					{ \
						.name = strview("value"), \
						.type = LODGE_TYPE_BOOL, \
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
	static bool node_select_ ## TYPE ## _run(struct lodge_node *node) \
	{ \
		const bool *value = lodge_node_get_boolean(node, 2); \
		if(!value) { \
			return false; \
		} \
		const C_TYPE *selected = lodge_node_get_ ## TYPE (node, *value ? 0 : 1); \
		if(!selected) { \
			return false; \
		} \
		lodge_node_set_ ## TYPE (node, 0, *selected); \
		return true; \
	}

#define lodge_node_type_select_register( TYPE ) \
	lodge_node_type_register(strview("select::" # TYPE), &node_select_ ## TYPE ## _configure, &node_select_ ## TYPE ## _run)

lodge_node_type_select_decl(i32, int32_t, LODGE_TYPE_I32)
lodge_node_type_select_decl(u32, uint32_t, LODGE_TYPE_U32)
lodge_node_type_select_decl(f32, float, LODGE_TYPE_F32)
lodge_node_type_select_decl(f64, double, LODGE_TYPE_F64)

static void lodge_ns_node_types_select_register()
{
	lodge_node_type_select_register(i32);
	lodge_node_type_select_register(u32);
	lodge_node_type_select_register(f32);
	lodge_node_type_select_register(f64);
}