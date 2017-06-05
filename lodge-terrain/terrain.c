
#include <stdint.h>
#include <math.h>

#include "math4.h"
#include "quadtree.h"
#include "geometry.h"
#include "intersect.h"
#include "alist.h"
#include "array.h"

#define TERRAIN_QUADTREE_LEAF_SIZE	32

static void ray_point(vec3 dst, const struct ray *ray, float t)
{
	dst[0] = ray->dir[0] * t;
	dst[1] = ray->dir[1] * t;
	dst[2] = ray->dir[2] * t;
	add3v(dst, ray->origin, dst);
}

struct terrain {
	struct quadtree		qtree;
	uint8_t				*heightmap;
	int					heightmap_width;
	int					heightmap_height;
};

struct terrain_quadtree_intersection {
	int		node_index;
	float	t_min;
	float	t_max;
};

void terrain_init(struct terrain *terrain, uint8_t *heightmap, int heightmap_width, int heightmap_height)
{
	terrain->heightmap = heightmap;
	terrain->heightmap_width = heightmap_width;
	terrain->heightmap_height = heightmap_height;

	// TODO(TS): configurable leaf dimensions
	quadtree_build_from_image(&terrain->qtree, heightmap, max(heightmap_width, heightmap_height), TERRAIN_QUADTREE_LEAF_SIZE);
}

static int terrain_quadtree_intersection_sort(const struct terrain_quadtree_intersection *lhs,
		const struct terrain_quadtree_intersection *rhs)
{
	return (lhs->t_min - rhs->t_min);
}

static void terrain_quadtree_vs_ray(const struct quadtree *qtree, const struct ray *ray, struct array *intersected)
{
	struct alist *queue = alist_new(128);

	alist_append(queue, 0);

	while(!alist_empty(queue)) {
		int index = (int)alist_delete_at(queue, 0, 0);
		struct quadtree_node *node = &qtree->nodes[index];

		struct aabb node_aabb = {
			.min = {
				node->area.min_x,
				node->area.min_y,
				node->val_min
			},
			.max = {
				node->area.max_x,
				node->area.max_y,
				node->val_max
			}
		};

		float t_max = -1;
		float t_min = -1;
		int is_intersected = intersect_ray_vs_aabb(ray, &node_aabb, &t_min, &t_max);

		if(is_intersected) {
			if(quadtree_is_leaf(qtree, index)) {
				struct terrain_quadtree_intersection result = {
					.node_index = index,
					.t_min = t_min,
					.t_max = t_max,
				};
				array_append(intersected, &result);
			} else {
				int first_child = quadtree_first_child(index);
				for(int i=0; i<4; i++) {
					alist_append(queue, (void*)(first_child + i));
				}
			}
		}
	}

	array_sort(intersected, &terrain_quadtree_intersection_sort);

	alist_free(queue, 0);
}

static int terrain_vs_ray_triangles_at(const struct terrain* terrain, int x, int y, float *t)
{
	// TODO(TS): write
	return 0;
}

static int terrain_vs_ray_bresenham(const struct terrain *terrain, int x0, int y0, int x1, int y1, float *t)
{
	int dx = abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2;
	int e2;

	for(;;) {
		if(terrain_vs_ray_triangles_at(terrain, x0, y0, t)) {
			return 1;
		}

		if(x0 == x1 && y0 == y1) {
			return 0;
		}

		e2 = err;

		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}

		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

int terrain_vs_ray(const struct terrain *terrain, const struct ray *ray, float *t)
{
	struct array *intersected = array_create_type(struct terrain_quadtree_intersection, 128);

	// Find intersected quadtree leafs
	terrain_quadtree_vs_ray(&terrain->qtree, ray, intersected);

	// Walk Bresenhams line between {t_min, t_max} in each leaf
	array_foreach(intersected, struct terrain_quadtree_intersection, it) {
		vec3 entry;
		vec3 exit;
		ray_point(entry, ray, it->t_min);
		ray_point(exit, ray, it->t_max);

		int x0 = floorf(entry[0]);
		int y0 = floorf(entry[1]);
		int x1 = ceilf(exit[0]);
		int y1 = ceilf(exit[1]);

		if(terrain_vs_ray_bresenham(terrain, x0, y0, x1, y1, t)) {
			break;
		}
	}

	array_destroy(intersected);
}
