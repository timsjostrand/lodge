#ifndef _LODGE_DRAW_CALL_H
#define _LODGE_DRAW_CALL_H

#include "lodge.h"

#define LODGE_DRAW_CALL_TEXTURE_SLOTS_MAX	64
#define LODGE_DRAW_CALL_UNIFORMS_MAX		64
#define LODGE_DRAW_CALL_DRAWABLES_MAX		64

enum lodge_type
{
	LODGE_TYPE_VEC2,
	LODGE_TYPE_VEC3,
	LODGE_TYPE_VEC4,
	LODGE_TYPE_IVEC2,
	LODGE_TYPE_IVEC3,
	LODGE_TYPE_IVEC4,
	LODGE_TYPE_MAT3x3,
	LODGE_TYPE_MAT4x4,
	LODGE_TYPE_USER,
};

struct lodge_texture_slot
{
	strview_t					name;
	struct lodge_sampler		sampler;
	struct lodge_texture		texture;
};

struct lodge_uniform_type
{
	enum lodge_type				type;
	uint32_t					size;
};

struct lodge_uniform
{
	strview_t					name;
	const void					*data;
	struct lodge_uniform_type	type;
};

struct lodge_draw_call
{
	struct lodge_shader			shader;
	struct lodge_texture_slot	texture_slots[LODGE_DRAW_CALL_TEXTURE_SLOTS_MAX];
	struct lodge_uniform		uniforms[LODGE_DRAW_CALL_UNIFORMS_MAX];
	struct drawable				drawables[LODGE_DRAW_CALL_DRAWABLES_MAX];
};

struct lodge_draw_call			lodge_draw_call_make(struct lodge_shader *shader, struct lodge_texture_slot texture_slots[], struct lodge_uniform uniforms[], struct drawable drawable);
struct lodge_texture_slot		lodge_texture_slot_make(strview_t name, struct lodge_sampler sampler, struct lodge_texture texture);
struct lodge_uniform			lodge_uniform_make(strview_t name, const void *data, struct lodge_uniform_type);

#if 0

struct lodge_draw_call my_draw_call = {
	.shader = assets.my_shader,
	.texture_slots = {
		lodge_texture_slot_make(strview_static("albedo"), linear_sampler, assets.foo.albedo),
		lodge_texture_slot_make(strview_static("specular"), linear_sampler, assets.foo.specular),
		lodge_texture_slot_make(strview_static("displacement"), linear_sampler, assets.foo.displacement),
	},
	.uniforms = {
		lodge_make_uniform(),
		lodge_camera_make_uniform(&game.camera),
		lodge_uniform_make(strview_static("my_color"), &game.color, LODGE_TYPE_VEC4),
		lodge_uniform_make(strview_static("intenseness"), &game.color, LODGE_TYPE_VEC4),
	},
	.drawable = &my_cube
};

lodge_renderer_draw(my_drawable, my_draw_call);

#endif

#endif