//
// Lodge Node System
//
#ifndef _LODGE_NS_H
#define _LODGE_NS_H

#include <stdint.h>

struct lodge_node_type;
typedef struct lodge_node_type* lodge_node_type_t;

struct lodge_node;
typedef struct lodge_node* lodge_node_t;

struct lodge_graph;
typedef struct lodge_graph* lodge_graph_t;

typedef uint16_t lodge_node_id_t;
typedef uint16_t lodge_pin_idx_t;

#endif