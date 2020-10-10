#ifndef _LODGE_NS_GRAPH_H
#define _LODGE_NS_GRAPH_H

#include "lodge_ns.h"

#include <stdbool.h>

//
// TODO:
//
//		- Separate Node Graph and RT Graph (runtime)
//		- We can do expensive operations on the Node Graph and treat it more
//		  of a description of the actual graph.
//		- The RT Graph is an optimized variant of the Node Graph. In this one
//		  we make things fast, like pre-allocating memory, storing pointers
//		  to cached data, removing unused nodes and optimizing constant expressions.
//		- To generate an RT Graph we call lodge_graph_configure() -- this will
//		  internally generate the RT Graph.
//		- Once configured, `lodge_graph_unconfigured` must be called to free
//		  the RT resources. After that, a new RT Graph may be generated.
//		- A Node Graph is not "runnable".
//		- An RT Graph is "runnable".
//		- Editors and validators will work on the Node Graph.
//		- Game will run RT Graphs.
//		- The RT Graph is only accessible from internal APIs such as actual
//		  node implementations.
//		- Not all node systems may want a Runtime. Text generators (such as a
//		  shader editor), probably just has a Node Graph.
//

lodge_graph_t	lodge_graph_new(void *context);
void			lodge_graph_free(lodge_graph_t graph);

void			lodge_graph_add_main(lodge_graph_t graph, lodge_node_t node);
bool			lodge_graph_remove_main(lodge_graph_t graph, lodge_node_t node);

bool			lodge_graph_configure(lodge_graph_t graph);
bool			lodge_graph_unconfigure(lodge_graph_t graph);
bool			lodge_graph_update(lodge_graph_t graph);

lodge_node_t	lodge_graph_add_node(lodge_graph_t graph, lodge_node_type_t type);
bool			lodge_graph_remove_node(lodge_graph_t graph, lodge_node_id_t node_id);

void*			lodge_graph_get_context(lodge_graph_t graph);
bool			lodge_graph_is_configured(lodge_graph_t graph);

size_t			lodge_graph_get_node_count(lodge_graph_t graph);
lodge_node_t	lodge_graph_get_node_from_index(lodge_graph_t graph, size_t index);
lodge_node_id_t	lodge_graph_get_node_id(lodge_graph_t graph, lodge_node_t node);

lodge_node_t	lodge_graph_find_node_from_id(lodge_graph_t graph, lodge_node_id_t node_id);

// FIXME(TS): not public
size_t			lodge_graph_get_node_index(lodge_graph_t graph, lodge_node_t node);

#endif