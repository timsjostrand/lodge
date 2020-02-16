#ifndef _LODGE_RENDERER_H
#define _LODGE_RENDERER_H

#include "lodge_plugin.h"

struct lodge_renderer;
struct lodge_sampler;
struct lodge_texture;
struct drawable;

typedef struct lodge_texture *texture_t;

typedef void (*lodge_render_func)(struct lodge_renderer *renderer, void *userdata);

struct lodge_renderer_alpha_blend_state
{
	int	enabled;
	int	src_blend;
	int dst_blend;
};

enum lodge_renderer_cull_face
{
	LODGE_RENDERER_CULL_FACE_DISABLE,
	LODGE_RENDERER_CULL_FACE_FRONT,
	LODGE_RENDERER_CULL_FACE_BACK,
	LODGE_RENDERER_CULL_FACE_FRONT_AND_BACK,
};

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

void					lodge_renderer_add_func(struct lodge_renderer *renderer, lodge_render_func render_func, void *userdata);
struct lodge_ret		lodge_renderer_attach(struct lodge_renderer *renderer);

strview_t				lodge_renderer_get_library(struct lodge_renderer *renderer);

int						lodge_renderer_set_clear_color(struct lodge_renderer *renderer, vec4 color);
int						lodge_renderer_set_cull_face(struct lodge_renderer *renderer, enum lodge_renderer_cull_face cull_face);
int						lodge_renderer_set_alpha_blend_state(struct lodge_renderer *renderer, struct lodge_renderer_alpha_blend_state state);

struct drawable*		lodge_renderer_get_unit_rect(struct lodge_renderer *renderer);

void					lodge_renderer_bind_shader(struct shader *shader);
void					lodge_renderer_bind_texture_unit(int slot, texture_t texture, struct lodge_sampler *sampler);
void					lodge_renderer_bind_sampler(int slot, struct lodge_sampler *sampler);
void					lodge_renderer_bind_texture(int slot, texture_t texture);

void					lodge_renderer_set_constant_vec2(struct shader *shader, strview_t name, vec2 v);
void					lodge_renderer_set_constant_vec3(struct shader *shader, strview_t name, vec3 v);
void					lodge_renderer_set_constant_vec4(struct shader *shader, strview_t name, vec4 v);
void					lodge_renderer_set_constant_mat4(struct shader *shader, strview_t name, mat4 mat);
void					lodge_renderer_set_constant_mvp(struct shader *shader, const struct mvp *mvp);
#if 0
void					lodge_renderer_draw(struct lodge_draw_call *draw_call);
#endif

#endif