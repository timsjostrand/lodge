#pragma once

#include "math4.h"

//
// TODO(TS): use alignas() instead of manual padding
//
struct lodge_camera_params
{
	vec3		pos;
	float		_pad;
	//vec3		dir;
	mat4		view;
	mat4		projection;
	mat4		view_projection;
	mat4		inv_view;
	mat4		inv_projection;
	mat4		inv_view_projection;
	float		fov_y;
	mat4		rotation;
};
