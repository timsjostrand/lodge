#include "lodge_ns_graph.h"

#include "lodge_ns_graph_types.h"
#include "lodge_ns_node.h"
#include "lodge_ns_node_type.h"

#include "membuf.h"
#include "lodge_assert.h"

#include <stdlib.h>

static int lodge_graph_node_id_compare(const void *lhs, const void *rhs)
{
	const struct lodge_node *lhs_node = lhs;
	const struct lodge_node *rhs_node = rhs;
	return lhs_node->id - rhs_node->id;
}

static int64_t lodge_graph_find_main_index(lodge_graph_t graph, lodge_node_t node)
{
	return membuf_find(membuf_wrap(graph->mains), graph->mains_count, node, sizeof(lodge_node_t));
}

static bool lodge_graph_remove_main_index(lodge_graph_t graph, size_t index)
{
	if(index < 0) {
		return false;
	}
	bool deleted = membuf_delete(membuf_wrap(graph), &graph->mains_count, (size_t)index, 1) == 1;
	ASSERT(deleted);
	return deleted;
}

lodge_graph_t lodge_graph_new(void *context)
{
	lodge_graph_t graph = (lodge_graph_t)calloc(1, sizeof(struct lodge_graph));
	graph->context = context;
	return graph;
}

void lodge_graph_free(lodge_graph_t graph)
{
	for(size_t i = 0; i < graph->nodes_count; i++) {
		lodge_node_reset(&graph->nodes[i]);
	}
	free(graph);
}

bool lodge_graph_unconfigure(lodge_graph_t graph)
{
	//ASSERT(graph->configured);
	graph->configured = false;
	return true;
}

bool lodge_graph_configure(lodge_graph_t graph)
{
	if(graph->configured) {
		lodge_graph_unconfigure(graph);
	}

	for(size_t i = 0, count = graph->nodes_count; i < count; i++) {
		lodge_node_t node = &graph->nodes[i];
		if(!lodge_node_type_configure_node(node->type, node)) {
			return false;
		}
	}

	graph->configured = true;
	return true;
}

bool lodge_graph_update(lodge_graph_t graph)
{
	if(!graph->configured) {
		return false;
	}
	if(graph->mains_count == 0) {
		return false;
	}
    for(size_t i = 0, count = graph->mains_count; i < count; i++) {
		if(!lodge_node_update(graph->mains[i])) {
			return false;
		}
	}
	return true;
}

void lodge_graph_add_main(lodge_graph_t graph, lodge_node_t node)
{
	ASSERT(!graph->configured);
	ASSERT(node->graph == graph);
	membuf_append(membuf_wrap(graph->mains), &graph->mains_count, &node, sizeof(lodge_node_t));
}

bool lodge_graph_remove_main(lodge_graph_t graph, lodge_node_t node)
{
	ASSERT(!graph->configured);
	ASSERT(node->graph == graph);
	const int64_t index = lodge_graph_find_main_index(graph, node);
	if(index < 0) {
		return false;
	}
	return lodge_graph_remove_main_index(graph, index);
}

lodge_node_t lodge_graph_add_node(lodge_graph_t graph, lodge_node_type_t type)
{
	ASSERT(graph);
	ASSERT(type);
	ASSERT(!graph->configured);

	lodge_node_t node = membuf_append(membuf_wrap(graph->nodes), &graph->nodes_count,
		&(struct lodge_node) {
			.id = graph->last_node_id++,
			.graph = graph,
			.type = type,
			.inputs = { 0 },
			.outputs = { 0 },
			.config = { 0 },
		},
		sizeof(struct lodge_node)
	);

	lodge_node_type_configure_node(node->type, node);
	
	return node;
}

size_t lodge_graph_get_node_index(lodge_graph_t graph, lodge_node_t node)
{
	return node - graph->nodes;
}

bool lodge_graph_remove_node(lodge_graph_t graph, lodge_node_id_t node_id)
{
	ASSERT(!graph->configured);

	// TODO(TS): how to repair node handles
	// - external APIs should never return node_t, but always be node_id_t
	// - internally, node_t can be used but must be updated on remove

	lodge_node_t node = lodge_graph_find_node_from_id(graph, node_id);
	if(!node) {
		return false;
	}

	//
	// Update mains list to account for modified memory layout.
	//
	{
		const int64_t main_index = lodge_graph_find_main_index(graph, node);
		if(main_index >= 0) {
			lodge_graph_remove_main_index(graph, main_index);
		}

		//
		// FIXME(TS): should be node_id_t instead, converted on RT Graph configure.
		//
		for(size_t main_idx = 0; main_idx < graph->mains_count; main_idx++) {
			if(graph->mains[main_idx] > node) {
				graph->mains[main_idx]--;
			}
		}
	}

	//
	// Update graph links to account for modified memory layout.
	//
	for(size_t node_idx = 0; node_idx < graph->nodes_count; node_idx++) {
		lodge_node_t other_node = &graph->nodes[node_idx];
		if(other_node == node) {
			continue;
		}

		for(lodge_pin_idx_t pin_idx = 0; pin_idx < other_node->inputs.count; pin_idx++) {
			struct lodge_input_pin *input_pin = &other_node->inputs.pins[pin_idx];

			if(input_pin->connection.node == node) {
				lodge_node_input_disconnect(other_node, pin_idx);
			}
			else if(input_pin->connection.node > node) {
				//
				// FIXME(TS): connections should hold node_id_t instead, and they should
				// be converted to ptrs on the RT Graph configure.
				//
				input_pin->connection.node--;
			}
		}
	}

	const size_t node_index = lodge_graph_get_node_index(graph, node);
	return membuf_delete(membuf_wrap(graph->nodes), &graph->nodes_count, node_index, 1) == 1;
}

void* lodge_graph_get_context(lodge_graph_t graph)
{
	return graph->context;
}

bool lodge_graph_is_configured(lodge_graph_t graph)
{
	return graph->configured;
}

size_t lodge_graph_get_node_count(lodge_graph_t graph)
{
	return graph->nodes_count;
}

lodge_node_t lodge_graph_get_node_from_index(lodge_graph_t graph, size_t index)
{
	return &graph->nodes[index];
}

lodge_node_id_t lodge_graph_get_node_id(lodge_graph_t graph, lodge_node_t node)
{
	return node->id;
}

lodge_node_t lodge_graph_find_node_from_id(lodge_graph_t graph, lodge_node_id_t node_id)
{
	return bsearch(
		&(struct lodge_node) { .id = node_id },
		graph->nodes,
		graph->nodes_count,
		sizeof(struct lodge_node),
		lodge_graph_node_id_compare
	);
}
