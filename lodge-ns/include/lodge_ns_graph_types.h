#ifndef _LODGE_NS_GRAPH_TYPES_H
#define _LODGE_NS_GRAPH_TYPES_H

#include "lodge_ns_node.h"

#include <stddef.h>

struct lodge_graph
{
	lodge_node_id_t							last_node_id;
	bool									configured;

	struct lodge_node                       nodes[255];
	size_t                                  nodes_count;

	struct lodge_node                       *mains[255];
	size_t									mains_count;

	void*									context;
};

#endif