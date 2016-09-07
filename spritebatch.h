#ifndef _SPRITEBATCH_H
#define _SPRITEBATCH_H

#include "math4.h"
#include "graphics.h"

#define SPRITEBATCH_BUFFER_CAPACITY 30720000
#define SPRITEBATCH_CHUNK 3072000
#define SPRITEBATCH_VERTEX_SIZE (5 * sizeof(GLfloat))

typedef int(*spritebatch_sort_fn)(GLfloat* buffer_data_a, GLfloat* buffer_data_b);

struct spritebatch
{
	GLuint texture;

	GLuint vbo;
	GLuint vao;

	GLuint offset_stream;
	GLuint offset_draw;

	unsigned int sprite_count;

	GLfloat* gpu_vertices;
};

void spritebatch_create(struct spritebatch* batch);
void spritebatch_destroy(struct spritebatch* batch);
void spritebatch_begin(struct spritebatch* batch);
void spritebatch_add(struct spritebatch* batch, vec3 pos, vec2 scale, vec2 tex_pos, vec2 tex_bounds);
void spritebatch_end(struct spritebatch* batch);
void spritebatch_render(struct spritebatch* batch, struct shader *s, struct graphics *g, GLuint tex, mat4 transform);
void spritebatch_sort(struct spritebatch* batch, spritebatch_sort_fn sorting_function);

#endif //_SPRITEBATCH_H
