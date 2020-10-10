#ifndef _LODGE_NS_GRAPH_SERIALIZE_H
#define _LODGE_NS_GRAPH_SERIALIZE_H

#include "lodge_ns.h"

#include "strview.h"

char*			lodge_graph_write(lodge_graph_t graph, size_t *size_out);
lodge_graph_t	lodge_graph_read(strview_t data, void *graph_context);

#endif