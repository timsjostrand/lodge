#include "lodge_quadtree.h"

#include "lodge_assert.h"

static void lodge_quadtree_make_impl(uint32_t level, const vec2 center, const vec2 size, lodge_quadtree_leaf_func_t is_leaf, void *userdata)
{
	ASSERT(is_leaf);

	if(!is_leaf(level, center, size, userdata)) {
		const vec2 size_half = { .x = size.x / 2.0f, .y = size.y / 2.0f };
		const vec2 offset = { .x = size_half.x / 2.0f, .y = size_half.y / 2.0f };

		// NE
		lodge_quadtree_make_impl(level + 1, (vec2) { .x = center.x + offset.x, .y = center.y + offset.y }, size_half, is_leaf, userdata);

		// NW
		lodge_quadtree_make_impl(level + 1, (vec2) { .x = center.x - offset.x, .y = center.y + offset.y }, size_half, is_leaf, userdata);

		// SW
		lodge_quadtree_make_impl(level + 1, (vec2) { .x = center.x - offset.x, .y = center.y - offset.y }, size_half, is_leaf, userdata);

		// SE
		lodge_quadtree_make_impl(level + 1, (vec2) { .x = center.x + offset.x, .y = center.y - offset.y }, size_half, is_leaf, userdata);
	}
}

void lodge_quadtree_make(const vec2 center, const vec2 size, lodge_quadtree_leaf_func_t is_leaf, void *userdata)
{
	lodge_quadtree_make_impl(0, center, size, is_leaf, userdata);
}
