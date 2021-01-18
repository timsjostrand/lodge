#ifndef _LODGE_QUADTREE_H
#define _LODGE_QUADTREE_H

#include "math4.h"

#include <stdbool.h>
#include <stdint.h>

//
// If this function returns false, it means the node is a leaf.
//
typedef bool (*lodge_quadtree_leaf_func_t)(const uint32_t level, const vec2 center, const vec2 size, void *userdata);

void lodge_quadtree_make(const vec2 center, const vec2 size, lodge_quadtree_leaf_func_t is_leaf, void *userdata);

#endif