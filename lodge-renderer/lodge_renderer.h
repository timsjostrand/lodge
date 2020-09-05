#ifndef _LODGE_RENDERER_H
#define _LODGE_RENDERER_H

#include "lodge_plugin.h"
#include "lodge_sampler.h"

struct lodge_renderer;
struct drawable;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

enum lodge_renderer_primitive
{
	LODGE_RENDERER_PRIMITIVE_POINTS,
	LODGE_RENDERER_PRIMITIVE_LINE_STRIP,
	LODGE_RENDERER_PRIMITIVE_LINE_LOOP,
	LODGE_RENDERER_PRIMITIVE_LINES,
	LODGE_RENDERER_PRIMITIVE_LINE_STRIP_ADJACENCY,
	LODGE_RENDERER_PRIMITIVE_LINES_ADJACENCY,
	LODGE_RENDERER_PRIMITIVE_TRIANGLE_STRIP,
	LODGE_RENDERER_PRIMITIVE_TRIANGLE_FAN,
	LODGE_RENDERER_PRIMITIVE_TRIANGLES,
	LODGE_RENDERER_PRIMITIVE_TRIANGLE_STRIP_ADJACENCY,
	LODGE_RENDERER_PRIMITIVE_TRIANGLES_ADJACENCY,
	LODGE_RENDERER_PRIMITIVE_PATCHES
};

typedef void* lodge_window_t;

struct lodge_renderer*	lodge_renderer_new();
void					lodge_renderer_free(struct lodge_renderer *renderer);

struct lodge_ret		lodge_renderer_attach(struct lodge_renderer *renderer);

strview_t				lodge_renderer_get_library(struct lodge_renderer *renderer);

struct drawable*		lodge_renderer_get_unit_rect(struct lodge_renderer *renderer);

void					lodge_renderer_bind_shader(lodge_shader_t shader);
void					lodge_renderer_bind_texture_unit(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler, enum lodge_texture_target target);
void					lodge_renderer_bind_sampler(int slot, const lodge_sampler_t sampler);
void					lodge_renderer_bind_texture(int slot, const lodge_texture_t texture, enum lodge_texture_target target);

void					lodge_renderer_bind_texture_unit_2d(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler);
void					lodge_renderer_bind_texture_2d(int slot, const lodge_texture_t texture);
void					lodge_renderer_bind_texture_unit_cube_map(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler);
void					lodge_renderer_bind_texture_cube_map(int slot, const lodge_texture_t texture);

void					lodge_renderer_set_constant_float(lodge_shader_t shader, strview_t name, float f);
void					lodge_renderer_set_constant_vec2(lodge_shader_t shader, strview_t name, vec2 v);
void					lodge_renderer_set_constant_vec3(lodge_shader_t shader, strview_t name, vec3 v);
void					lodge_renderer_set_constant_vec4(lodge_shader_t shader, strview_t name, vec4 v);
void					lodge_renderer_set_constant_mat4(lodge_shader_t shader, strview_t name, mat4 mat);
void					lodge_renderer_set_constant_mvp(lodge_shader_t shader, const struct mvp *mvp);

#endif
