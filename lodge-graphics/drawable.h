#ifndef _DRAWABLE_H
#define _DRAWABLE_H

#include "math4.h"
#include "shader.h"
#include "graphics.h"
#include "geometry.h"
#include "graphics_types.h"

struct drawable {
	enum draw_mode	draw_mode;
	uint32_t		vertex_count;
	uint32_t		vbo;
	uint32_t		vao;
};

struct vertex_buffer;

struct drawable		drawable_make(enum draw_mode draw_mode, uint32_t vertex_count, uint32_t vbo, uint32_t vao);
struct drawable		drawable_make_from_buffer(struct vertex_buffer *vb, enum draw_mode mode);
void				drawable_reset(struct drawable *d);

void				drawable_new_circle_outline(struct drawable *dst, struct circle *circle, int segments, struct shader *s);
void				drawable_new_circle_outlinef(struct drawable *dst, float x, float y, float r, int segments, struct shader *s);

void				drawable_new_rect_outline(struct drawable *dst, struct rect *rect, struct shader *s);
void				drawable_new_rect_outlinef(struct drawable *dst, float x, float y, float w, float h, struct shader *s);
void				drawable_new_rect_solidf(struct drawable *dst, float x, float y, float w, float h, struct shader *s);
void				drawable_new_rect_fullscreen(struct drawable *dst, struct shader *s);

struct drawable		drawable_make_line(vec3 start, vec3 end);
struct drawable		drawable_make_unit_cube();

struct drawable		drawable_make_plane_subdivided(vec2 origin, vec2 size, int divisions_x, int divisions_y);
struct drawable		drawable_make_plane_subdivided_vertex(vec2 origin, vec2 size, int divisions_x, int divisions_y);

void				drawable_render(struct drawable *d);
void				drawable_render_detailed(enum draw_mode mode, uint32_t vao, uint32_t vertex_count, GLuint *tex, vec4 color, struct shader *s, struct mvp mvp);
void				drawable_render_simple(struct drawable *d, struct shader *s, GLuint *tex, vec4 color, mat4 transform);

#endif
