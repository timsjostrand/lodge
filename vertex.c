#include "vertex.h"

xyzuv_t xyzuv_make(float x, float y, float z, float u, float v)
{
	xyzuv_t tmp = {
		.x = x,
		.y = y,
		.z = z,
		.u = u,
		.v = v
	};
	return tmp;
}

void vertex_calc_tangents(vertex_t *t1, vertex_t *t2, vertex_t *t3)
{
	vec3 edge1 = vec3_sub(t2->pos, t1->pos);
	vec3 edge2 = vec3_sub(t3->pos, t1->pos);
	vec2 delta_uv1 = vec2_sub(t2->uv, t1->uv);
	vec2 delta_uv2 = vec2_sub(t3->uv, t1->uv);

	float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);
	f = 1.0;

	vec3 tangent;
	tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
	tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
	tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
	tangent = vec3_norm(tangent);

	vec3 bitangent;
	bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
	bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
	bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
	bitangent = vec3_norm(bitangent);

	t1->tangent = tangent;
	t2->tangent = tangent;
	t3->tangent = tangent;

	t1->bitangent = bitangent;
	t2->bitangent = bitangent;
	t3->bitangent = bitangent;
}
