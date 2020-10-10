# lodge-ns

add_library(lodge-ns STATIC "")

target_sources(lodge-ns
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_graph_types.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_graph.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_graph_serialize.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node_type.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node_types_default.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_graph.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_graph_serialize.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node_type.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node_types_helpers.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node_types_default.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_ns_node.h"
)

target_include_directories(lodge-ns
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-ns
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-lib
		lodge-serialize-json
)
