/**
 * A T-pyramid quadtree (all leaf nodes are on same level) backed by an array.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>

#include "quadtree.h"
#include "math4.h"

int quadtree_is_leaf(const struct quadtree *qtree, int node_index)
{
	return node_index >= qtree->leaf_index_start;
}

int quadtree_parent_index(int child_index)
{
	return (child_index - 1) / 4;
}

int quadtree_first_child(int parent_index)
{
	return (parent_index * 4) + 1;
}

static void quadtree_populate_node(const struct quadtree *qtree, int node_index, int is_leaf, const uint8_t *image)
{
	struct quadtree_node *node = &qtree->nodes[node_index];

	int node_min = INT_MAX;
	int node_max = -INT_MAX;

	if(is_leaf) {
		for(int y=node->area.min_y, ymax=node->area.max_y; y<ymax; y++) {
			for(int x=node->area.min_x, xmax=node->area.max_x; x<xmax; x++) {
				uint8_t val = image[(qtree->dimensions - y - 1) * qtree->dimensions + x];
				node_min = min(val, node_min);
				node_max = max(val, node_max);
			}
		}
	} else {
		int child_index = quadtree_first_child(node_index);
		for(int i=0; i<4; i++) {
			node_min = min(qtree->nodes[child_index + i].val_min, node_min);
			node_max = max(qtree->nodes[child_index + i].val_max, node_max);
		}
	}

	node->val_min = node_min;
	node->val_max = node_max;
}

static int quadtree_nodes_count(int subdivisions)
{
	int nodes_count = 0;

	while(subdivisions >= 0) {
		nodes_count += powi(4, subdivisions);
		subdivisions--;
	}

	return nodes_count;
}

int quadtree_subdivisions_required(int parent_dimensions, int child_dimensions)
{
	return log2i(parent_dimensions / child_dimensions);
}

void quadtree_build_from_image(struct quadtree *qtree, const uint8_t *image, int image_dimensions, int leaf_dimensions)
{
	qtree->dimensions = image_dimensions;
	qtree->subdivisions = quadtree_subdivisions_required(qtree->dimensions, leaf_dimensions);
	qtree->nodes_count = quadtree_nodes_count(qtree->subdivisions);
	qtree->leaf_index_start = quadtree_nodes_count(qtree->subdivisions - 1);
	qtree->nodes = (struct quadtree_node *) malloc(qtree->nodes_count * sizeof(struct quadtree_node));

	struct quadtree_node *node = &qtree->nodes[0];
	node->area.min_x = 0;
	node->area.min_y = 0;
	node->area.max_x = image_dimensions;
	node->area.max_y = image_dimensions;
	node->val_min = -1;
	node->val_max = -1;

	for(int i=1, count=qtree->nodes_count; i<count; i+=4) {
		int parent_index = quadtree_parent_index(i);
		struct quadtree_node *parent = &qtree->nodes[parent_index];
		struct area *pa = &parent->area;
		int pa_half_x = pa->min_x + (pa->max_x - pa->min_x)/2;
		int pa_half_y = pa->min_y + (pa->max_y - pa->min_y)/2;

		// NW Child
		node = &qtree->nodes[i + 0];
		node->area.min_x = pa->min_x;
		node->area.min_y = pa_half_y;
		node->area.max_x = pa_half_x;
		node->area.max_y = pa->max_y;
		node->val_min = -1;
		node->val_max = -1;

		// NE Child
		node = &qtree->nodes[i + 1];
		node->area.min_x = pa_half_x;
		node->area.min_y = pa_half_y;
		node->area.max_x = pa->max_x;
		node->area.max_y = pa->max_y;
		node->val_min = -1;
		node->val_max = -1;

		// SW Child
		node = &qtree->nodes[i + 2];
		node->area.min_x = pa->min_x;
		node->area.min_y = pa->min_y;
		node->area.max_x = pa_half_x;
		node->area.max_y = pa_half_y;
		node->val_min = -1;
		node->val_max = -1;

		// SE Child
		node = &qtree->nodes[i + 3];
		node->area.min_x = pa_half_x;
		node->area.min_y = pa->min_y;
		node->area.max_x = pa->max_x;
		node->area.max_y = pa_half_y;
		node->val_min = -1;
		node->val_max = -1;
	}

	for(int i=qtree->nodes_count-1; i>=0; i--) {
		quadtree_populate_node(qtree, i, i >= qtree->leaf_index_start, image);
	}
}
