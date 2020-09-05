#ifndef _SPRITEBATCH_H
#define _SPRITEBATCH_H

#include "math4.h"

#define SPRITEBATCH_BUFFER_CAPACITY 30720000
#define SPRITEBATCH_CHUNK 3072000
#define SPRITEBATCH_VERTEX_SIZE (5 * sizeof(GLfloat))

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

typedef float GLfloat;
typedef unsigned int GLuint;
typedef struct lodge_texture* lodge_texture_t;
typedef int(*spritebatch_sort_fn)(const GLfloat* buffer_data_a, const GLfloat* buffer_data_b);

struct spritebatch
{
	GLuint vbo;
	GLuint vao;

	GLuint offset_stream;
	GLuint offset_draw;

	unsigned int sprite_count;

	GLfloat* gpu_vertices;
};

struct spritebatch* spritebatch_create();
void spritebatch_destroy(struct spritebatch* batch);
void spritebatch_begin(struct spritebatch* batch);
void spritebatch_add(struct spritebatch* batch, vec3 pos, vec2 scale, vec2 tex_pos, vec2 tex_bounds);
void spritebatch_end(struct spritebatch* batch);
void spritebatch_sort(struct spritebatch* batch, spritebatch_sort_fn sorting_function);

void spritebatch_render(struct spritebatch* batch, lodge_shader_t s);
void spritebatch_render_simple(struct spritebatch* batch, lodge_shader_t s, lodge_texture_t texture, mat4 projection, mat4 transform);

#endif //_SPRITEBATCH_H
