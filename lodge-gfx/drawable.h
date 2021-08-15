//
// DEPRECATED
//
// DO NOT USE, SEE `lodge_drawable.h`
//
#ifndef _DRAWABLE_H
#define _DRAWABLE_H

#include "math4.h"
#include "geometry.h"
#include "lodge_gfx.h"
#include "lodge_shader.h"

struct drawable
{
	enum lodge_gfx_primitive		primitive;
	uint32_t						vertex_count;
	uint32_t						vbo;
	uint32_t						vao;
};

struct vertex_buffer;

struct drawable		drawable_make(enum lodge_gfx_primitive primitive, uint32_t vertex_count, uint32_t vbo, uint32_t vao);
#if 0
struct drawable		drawable_make_from_buffer(struct vertex_buffer *vb, enum lodge_gfx_primitive mode);
#endif
void				drawable_reset(struct drawable *d);

#if 0
void				drawable_new_circle_outline(struct drawable *dst, struct circle *circle, int segments);
void				drawable_new_circle_outlinef(struct drawable *dst, float x, float y, float r, int segments);
#endif

void				drawable_new_rect_outline(struct drawable *dst, struct rect *rect);
void				drawable_new_rect_outlinef(struct drawable *dst, float x, float y, float w, float h);
void				drawable_new_rect_solidf(struct drawable *dst, float x, float y, float w, float h);
#if 0
void				drawable_new_rect_fullscreen(struct drawable *dst, struct shader *s);
#endif

#if 0
struct drawable		drawable_make_line(vec3 start, vec3 end);
struct drawable		drawable_make_unit_cube();
#endif

struct drawable		drawable_make_plane_subdivided(vec2 origin, vec2 size, int divisions_x, int divisions_y);
struct drawable		drawable_make_plane_subdivided_vertex(vec2 origin, vec2 size, int divisions_x, int divisions_y);

void				drawable_render(const struct drawable *d);
#if 0
void				drawable_render_detailed(enum lodge_gfx_primitive primitive, uint32_t vao, uint32_t vertex_count, GLuint *tex, vec4 color, struct shader *s, struct mvp mvp);
void				drawable_render_simple(struct drawable *d, struct shader *s, GLuint *tex, vec4 color, mat4 transform);
#endif

#endif
