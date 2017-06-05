#ifndef _QUADTREE_H
#define _QUADTREE_H

struct area {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
};

struct quadtree_node {
	struct area		area;
	int				val_min;
	int				val_max;
};

struct quadtree {
	int						dimensions;
	int						subdivisions;
	int						nodes_count;
	int						leaf_size;
	int						leaf_index_start;
	struct quadtree_node	*nodes;
};

void	quadtree_build_from_image(struct quadtree *qtree, const uint8_t *image, int image_dimensions, int leaf_dimensions);
int		quadtree_parent_index(int child_index);
int		quadtree_first_child(int parent_index);
int		quadtree_is_leaf(const struct quadtree *qtree, int node_index);

#endif
