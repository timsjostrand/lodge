#include "lodge_parametric_mesh.h"

#include "dynbuf.h"

static inline void lodge_parametric_new_inplace(struct lodge_parametric_mesh *dst, size_t count)
{
	dynbuf_new_inplace(dynbuf(dst->vertices), count);
	dynbuf_new_inplace(dynbuf(dst->normals), count);
	dynbuf_new_inplace(dynbuf(dst->uvs), count);
}

static inline void lodge_parametric_append(struct lodge_parametric_mesh *dst, vec3 vertex, vec3 normal, vec2 uv)
{
	dynbuf_append(dynbuf(dst->vertices), &vertex, sizeof(vec3));
	dynbuf_append(dynbuf(dst->normals), &normal, sizeof(vec3));
	dynbuf_append(dynbuf(dst->uvs), &uv, sizeof(vec2));
}

static inline void lodge_parametric_append_plane_quads(struct lodge_parametric_mesh *dst, float x, float y, float w, float h, bool mirror)
{
	static const vec3 n = { 0.0f, 0.0f, 1.0f };

	if (mirror) {
		lodge_parametric_append(dst, vec3_make(x, y + h, 0.0f),		n, vec2_make(0.0f, 0.0f)); /* Top-left */
		lodge_parametric_append(dst, vec3_make(x, y, 0.0f),			n, vec2_make(0.0f, 1.0f)); /* Bottom-Left */
		lodge_parametric_append(dst, vec3_make(x + w, y, 0.0f),		n, vec2_make(1.0f, 1.0f)); /* Bottom-right */
		lodge_parametric_append(dst, vec3_make(x + w, y + h, 0.0f),	n, vec2_make(1.0f, 0.0f)); /* Top-right */
		lodge_parametric_append(dst, vec3_make(x, y + h, 0.0f),		n, vec2_make(0.0f, 0.0f)); /* Top-left */
		lodge_parametric_append(dst, vec3_make(x + w, y, 0.0f),		n, vec2_make(1.0f, 1.0f)); /* Bottom-right */
	}
	else {
		lodge_parametric_append(dst, vec3_make(x, y + h, 0.0f),		n, vec2_make(0.0f, 0.0f)); /* Top-left */
		lodge_parametric_append(dst, vec3_make(x, y, 0.0f),			n, vec2_make(0.0f, 1.0f)); /* Bottom-Left */
		lodge_parametric_append(dst, vec3_make(x + w, y + h, 0.0f),	n, vec2_make(1.0f, 0.0f)); /* Top-right */
		lodge_parametric_append(dst, vec3_make(x + w, y + h, 0.0f),	n, vec2_make(1.0f, 0.0f)); /* Top-right */
		lodge_parametric_append(dst, vec3_make(x, y, 0.0f),			n, vec2_make(0.0f, 1.0f)); /* Bottom-left */
		lodge_parametric_append(dst, vec3_make(x + w, y, 0.0f),		n, vec2_make(1.0f, 1.0f)); /* Bottom-right */
	}
}

void lodge_parametric_mesh_free_inplace(struct lodge_parametric_mesh* dst)
{
	dynbuf_free_inplace(dynbuf(dst->vertices));
	dynbuf_free_inplace(dynbuf(dst->normals));
	dynbuf_free_inplace(dynbuf(dst->uvs));
}


struct lodge_parametric_mesh lodge_parametric_mesh_new_plane_subdivided(vec2 origin, vec2 size, uint32_t divisions_x, uint32_t divisions_y)
{
	const size_t vertex_count = 6 * divisions_x * divisions_y;

	struct lodge_parametric_mesh dst = { 0 };
	lodge_parametric_new_inplace(&dst, vertex_count);

	/* Calculate vertices. */
	const float w = size.x / divisions_x;
	const float h = size.y / divisions_y;
	for(uint32_t y = 0; y < divisions_y; y++) {
		for(uint32_t x = 0; x < divisions_x; x++) {
			const float x0 = origin.x + x * w;
			const float y0 = origin.y + y * h;
			const bool mirror = (y + x) % 2 == 0;
			lodge_parametric_append_plane_quads(&dst, x0, y0, w, h, mirror);
		}
	}

	return dst;
}
