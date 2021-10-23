#ifndef _COORDINATE_SYSTEMS_H
#define _COORDINATE_SYSTEMS_H

#include "math4.h"

#include <stdbool.h>

//
// Inline helpers to convert between coordinate systems, using the naming conventions from:
//
//		https://learnopengl.com/Getting-started/Coordinate-Systems
//
//	local_space * model				=> world_space
//	world_space * view				=> view_space
//	view_space * projection			=> clip_space
//	clip_space.xyz / clip_space.w	=> ndc_space
//	ndc_space * viewport			=> screen_space
//

inline vec3 vec4_clip_to_ndc_space(const vec4 clip_space)
{
	return vec3_div_scalar(vec3_make(xyz_of(clip_space)), clip_space.w);
}

inline vec3 vec3_ndc_to_world_space(const vec3 ndc_space, const mat4 *inv_view_projection)
{
	const vec4 clip_space = mat4_mult_vec4(*inv_view_projection, vec4_make(xyz_of(ndc_space), 1.0f));
	return vec4_clip_to_ndc_space(clip_space);
}

//
// Given some normalized screen coordinates:
// 
//		{ .x = [0..1], .y = [0..1] }
//
// , returns the corresponding world space coordinates.
//
inline vec3 vec2_screen_01_to_world_space(const vec2 screen_space, const mat4 *inv_view_projection)
{
	const vec3 ndc_space = vec3_make(screen_space.x*2.0f - 1.0f, screen_space.y*2.0f - 1.0f, 0.0f);
	return vec3_ndc_to_world_space(ndc_space, inv_view_projection);
}

// FIXME(TS): not tested
inline vec3 vec3_world_to_view_space(const vec3 world_space, const mat4 *view)
{
	const vec4 tmp = mat4_mult_vec4(*view, vec4_make_from_vec3(world_space, 1.0f));
	return vec3_make(xyz_of(tmp));
}

// FIXME(TS): not tested
inline vec3 vec3_world_to_ndc_space(const vec3 world_space, const mat4 *view_projection)
{
	const vec4 clip_space = mat4_mult_vec4(*view_projection, vec4_make_from_vec3(world_space, 1.0f));
	return vec4_clip_to_ndc_space(clip_space);
}

// FIXME(TS): not tested
inline bool vec3_is_in_clip(const vec3 clip_space)
{
	return clip_space.x >= -1.0f && clip_space.x <= 1.0f
		&& clip_space.y >= -1.0f && clip_space.y <= 1.0f;
}

#endif