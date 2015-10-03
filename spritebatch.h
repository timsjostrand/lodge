#ifndef _SPRITEBATCH_H
#define _SPRITEBATCH_H

#include "graphics.h"

#define SPRITEBATCH_BUFFER_CAPACITY 30720000
#define SPRITEBATCH_CHUNK 3072000
#define SPRITEBATCH_VERTEX_SIZE (5 * sizeof(GLfloat))

typedef float GLfloat;
typedef unsigned int GLuint;
typedef float mat4[16];
typedef float vec3[3];
typedef float vec2[2];

struct spritebatch
{
	GLuint texture;

	GLuint vbo;
	GLuint vao;

	GLuint offset_stream;
	GLuint offset_draw;

	GLfloat* gpu_vertices;

	unsigned int sprite_count;
};

void spritebatch_create(struct spritebatch* batch);
void spritebatch_destroy(struct spritebatch* batch);
void spritebatch_begin(struct spritebatch* batch);
void spritebatch_add(struct spritebatch* batch, vec3 pos, vec2 scale, vec2 tex_pos, vec2 tex_bounds);
void spritebatch_end(struct spritebatch* batch);
void spritebatch_render(struct spritebatch* batch, struct shader *s, struct graphics *g, GLuint tex);

#endif //_SPRITEBATCH_H
