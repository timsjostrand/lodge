#ifndef _LODGE_GFX_H
#define _LODGE_GFX_H

#include "strview.h"

#include "lodge_gfx_types.h"
#include "lodge_plugin.h"

struct lodge_gfx;
struct drawable;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_sampler;
typedef struct lodge_sampler* lodge_sampler_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

enum lodge_gfx_primitive
{
	LODGE_GFX_PRIMITIVE_POINTS,
	LODGE_GFX_PRIMITIVE_LINE_STRIP,
	LODGE_GFX_PRIMITIVE_LINE_LOOP,
	LODGE_GFX_PRIMITIVE_LINES,
	LODGE_GFX_PRIMITIVE_LINE_STRIP_ADJACENCY,
	LODGE_GFX_PRIMITIVE_LINES_ADJACENCY,
	LODGE_GFX_PRIMITIVE_TRIANGLE_STRIP,
	LODGE_GFX_PRIMITIVE_TRIANGLE_FAN,
	LODGE_GFX_PRIMITIVE_TRIANGLES,
	LODGE_GFX_PRIMITIVE_TRIANGLE_STRIP_ADJACENCY,
	LODGE_GFX_PRIMITIVE_TRIANGLES_ADJACENCY,
	LODGE_GFX_PRIMITIVE_PATCHES
};

#if 0
struct lodge_gfx_texture_unit
{
	enum lodge_texture_target			target;
	lodge_sampler_t						sampler;
	lodge_texture_t						texture;
};

struct lodge_gfx_texture_units
{
	size_t								count;
	struct lodge_gfx_texture_unit		elements[16];
};

struct lodge_gfx_uniform_buffer
{
	lodge_buffer_object_t				buffer_object;
	size_t								offset;
	size_t								size;
};

struct lodge_gfx_uniform_buffers
{
	size_t								count;
	struct lodge_gfx_uniform_buffer		elements[16];
};

struct lodge_gfx_bindings_desc
{
	lodge_shader_t						shader;
	struct lodge_gfx_texture_units		textures;
	struct lodge_gfx_uniform_buffers	uniform_buffers;
};
#endif

struct lodge_gfx*						lodge_gfx_new();
void									lodge_gfx_free(struct lodge_gfx *gfx);

struct lodge_ret						lodge_gfx_attach(struct lodge_gfx *gfx);

strview_t								lodge_gfx_get_library(struct lodge_gfx *gfx);

void									lodge_gfx_bind_shader(lodge_shader_t shader);
void									lodge_gfx_bind_texture_unit(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler, enum lodge_texture_target target);
void									lodge_gfx_bind_sampler(int slot, const lodge_sampler_t sampler);
void									lodge_gfx_bind_texture(int slot, const lodge_texture_t texture, enum lodge_texture_target target);

void									lodge_gfx_bind_texture_unit_2d(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler);
void									lodge_gfx_bind_texture_2d(int slot, const lodge_texture_t texture);

void									lodge_gfx_bind_texture_unit_3d(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler);

void									lodge_gfx_bind_texture_unit_cube_map(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler);
void									lodge_gfx_bind_texture_cube_map(int slot, const lodge_texture_t texture);

void									lodge_gfx_bind_texture_unit_2d_array(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler);

#if 0
void									lodge_gfx_set_bindings(struct lodge_gfx_bindings *bindings);
#endif

void									lodge_gfx_set_viewport(int32_t x, int32_t y, size_t width, size_t height);
void									lodge_gfx_set_scissor(int32_t x, int32_t y, size_t width, size_t height);

void									lodge_gfx_annotate_begin(strview_t message);
void									lodge_gfx_annotate_end();

#endif
