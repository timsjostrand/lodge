#ifndef _DRAWABLE_H
#define _DRAWABLE_H

#include "math4.h"
#include "shader.h"
#include "graphics.h"
#include "geometry.h"

struct drawable {
	GLenum	draw_mode;
	GLuint	vertex_count;
	GLuint	vbo;
	GLuint	vao;
};

void	drawable_free(struct drawable *d);

void	drawable_new_circle_outline(struct drawable *dst, struct circle *circle, int segments, struct shader *s);
void	drawable_new_circle_outlinef(struct drawable *dst, float x, float y, float r, int segments, struct shader *s);

void	drawable_new_rect_outline(struct drawable *dst, struct rect *rect, struct shader *s);
void	drawable_new_rect_outlinef(struct drawable *dst, float x, float y, float w, float h, struct shader *s);
void	drawable_new_rect_solidf(struct drawable *dst, float x, float y, float w, float h, struct shader *s);

void	drawable_new_linef(struct drawable *dst, float x1, float y1, float x2, float y2, struct shader *s);

void	drawable_render_detailed(GLenum mode, GLuint vbo, unsigned int vbo_count, GLuint vao, GLuint *tex, vec3 color,
				struct shader *s, struct graphics *g, mat4 transform);
void	drawable_render(struct drawable *d, struct shader *s, struct graphics *g, GLuint *tex, vec4 color, mat4 transform);

/* FIXME: basic_sprite should be refactored into a drawable of type "rect". */
struct basic_sprite {
	int		type;
	vec4	pos;
	vec4	scale;
	vec4	color;
	float	rotation;
	GLuint	*texture;
};

void	sprite_init(struct basic_sprite *sprite, int type, float x, float y, float z,
				float w, float h, const vec4 color, float rotation, GLuint *texture);
void	sprite_render(struct basic_sprite *sprite, struct shader *s, struct graphics *g);

#endif
