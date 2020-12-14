#ifndef _LODGE_TESSELATED_PLANE_H
#define _LODGE_TESSELATED_PLANE_H

#include "math4.h"

struct lodge_camera_params;

struct lodge_tesselated_plane;

struct lodge_debug_draw;

struct lodge_tesselated_plane_chunk
{
	vec2								center;
	vec2								size;
	uint8_t								lod;
};

void									lodge_tesselated_plane_new_inplace(struct lodge_tesselated_plane *plane);
void									lodge_tesselated_plane_free_inplace(struct lodge_tesselated_plane *plane);
size_t									lodge_tesselated_plane_sizeof();

void									lodge_tesselated_plane_update(struct lodge_tesselated_plane *plane, vec3 pos, vec3 scale, const struct lodge_camera_params *camera, uint32_t level_max, float lod_switch_distance, struct lodge_debug_draw *debug);

struct lodge_tesselated_plane_chunk*	lodge_tesselated_plane_chunks_begin(struct lodge_tesselated_plane *plane);
struct lodge_tesselated_plane_chunk*	lodge_tesselated_plane_chunks_end(struct lodge_tesselated_plane *plane);

#endif