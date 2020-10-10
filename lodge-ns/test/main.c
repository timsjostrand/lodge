#include "lodge_ns.h"
#include "lodge_ns_graph.h"
#include "lodge_ns_node_type.h"
#include "lodge_ns_node_types_default.h"
#include "lodge_ns_node.h"

void lodge_ns_test(int argc, char **argv)
{
	lodge_node_types_default_register();

	lodge_node_type_t type_const_int32 = lodge_node_type_find(strview_static("const::int32"));
	lodge_node_type_t type_math_add = lodge_node_type_find(strview_static("math::add::i32"));
	lodge_node_type_t type_i32_print = lodge_node_type_find(strview_static("util::print::i32"));

	lodge_graph_t graph = lodge_graph_new();

	lodge_node_t node_a = lodge_graph_add_node_const_int32(graph, 0);
	lodge_node_t node_b = lodge_graph_add_node_const_int32(graph, 1);
	lodge_node_t node_c = lodge_graph_add_node(graph, type_math_add);
	lodge_node_t node_d = lodge_graph_add_node(graph, type_i32_print);

	lodge_node_connect(node_c, 0, node_a, 0);
	lodge_node_connect(node_c, 1, node_b, 0);
	
	lodge_node_connect(node_d, 0, node_c, 0);

#if 0
    struct lodge_node node_c = {
        .type = type_math_add,
        .inputs = {
            .pins = {
                [0] = {
                    .connection = {
                        .node = &node_a,
                        .pin_index = 0,
                    },
                    .type = LODGE_PIN_TYPE_CONNECTION,
                },
                [1] = {
                    .connection = {
                        .node = &node_b,
                        .pin_index = 0,
                    },
                    .type = LODGE_PIN_TYPE_CONNECTION,
                },
            },
            .count = 2
        },
    };

    struct lodge_node node_d = {
        .type = type_i32_print,
        .inputs = {
            .pins = {
                [0] = {
                    .connection = {
                        .node = &node_c,
                        .pin_index = 0,
                    },
                    .type = LODGE_PIN_TYPE_CONNECTION,
                }
            },
            .count = 1
        }
    };
#endif

    if(lodge_graph_init(graph)) {
        for(int i = 0; i < 5; i++) {
            // node system specific inputs... delta_time etc?
            lodge_node_set_int32(node_a, 0, i);

            lodge_graph_run(graph);
        }
    }
}
