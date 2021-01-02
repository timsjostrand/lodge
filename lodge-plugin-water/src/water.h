#ifndef _WATER_H
#define _WATER_H

#include "math4.h"
#include "drawable.h"
#include "skybox.h"
#include "lodge_framebuffer.h"
#include "lodge_sampler.h"
#include "lodge_texture.h"

struct lodge_texture;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct water_textures
{
	lodge_texture_t	heightmap;
	lodge_texture_t	cubemap;
	lodge_texture_t	water_foam;
	lodge_texture_t	water_normals[2];
};

struct water_samplers
{
	lodge_sampler_t			linear_repeat;
	lodge_sampler_t			linear_clamp;
};

struct water
{
	struct drawable					drawable;
	struct water_textures			textures;
	struct water_samplers			samplers;

	vec3							*scale;
	vec3							*pos;
	vec3							*offset;
	vec3							wave_scale;
};

struct water_render_params
{
	float				time;
	vec3				camera_pos;
	vec3				terrain_scale;
	vec3				sun_dir;
	mat4				perspective;
	mat4				view;
	lodge_texture_t		depth_texture;
	lodge_texture_t		color_texture;
};

void water_init(struct water *water, struct water_textures textures, vec3 *pos, vec3 *scale, vec3 *offset);
void water_render(struct water *water, lodge_shader_t shader, const struct water_render_params params);

#endif