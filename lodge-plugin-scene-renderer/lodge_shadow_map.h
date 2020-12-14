#ifndef _LODGE_SHADOW_MAP_H
#define _LODGE_SHADOW_MAP_H

#include "math4.h"
#include "frustum.h"
#include "lodge_platform.h"

#define LODGE_SHADOW_CASCADE_SPLITS_COUNT 3

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

struct lodge_shadow_map_buffer
{
	mat4							views[LODGE_SHADOW_CASCADE_SPLITS_COUNT];
	mat4							projections[LODGE_SHADOW_CASCADE_SPLITS_COUNT];
	vec4							cascade_splits;
	vec4							cascade_splits_worldspace;
};

struct lodge_shadow_map
{
	uint32_t						width;
	uint32_t						height;
	float							far_factor;
	float							near_factor;
	lodge_framebuffer_t				framebuffer;
	lodge_texture_t					depth_textures_array;
	struct lodge_shadow_map_buffer	buffer;
	lodge_buffer_object_t			buffer_object;

};

struct lodge_shadow_map_debug
{
	vec3							light_pos;
	vec3							frustum_center;
	vec3							p_max;
	vec3							p_min;
	struct frustum_corners			frustum;
	struct aabb						frustum_bounds;
	lodge_texture_t					texture;
};

void								lodge_shadow_map_new_inplace(struct lodge_shadow_map *shadow_map, uint32_t width, uint32_t height, float z_near, float z_far);
void								lodge_shadow_map_update(struct lodge_shadow_map *shadow_map, float dt, const mat4 inv_view_proj, vec3 light_dir, struct lodge_shadow_map_debug debug_out[LODGE_SHADOW_CASCADE_SPLITS_COUNT]);

#endif