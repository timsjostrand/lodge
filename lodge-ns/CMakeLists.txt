# lodge-ns

add_library(lodge-ns STATIC "")

target_sources(lodge-ns
	PRIVATE
		"src/lodge_ns_graph.c"
		"src/lodge_ns_graph_serialize.c"
		"src/lodge_ns_node_type.c"
		"src/lodge_ns_node_types_default.c"
		"src/lodge_ns_node.c"
	PUBLIC
		"include/lodge_ns.h"
		"include/lodge_ns_graph.h"
		"include/lodge_ns_graph_serialize.h"
		"include/lodge_ns_node_type.h"
		"include/lodge_ns_node_types_helpers.h"
		"include/lodge_ns_node_types_default.h"
		"include/lodge_ns_node.h"
)

target_include_directories(lodge-ns
	PUBLIC
		"include/"
)

target_link_libraries(lodge-ns
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-lib
		lodge-serialize-json
)
