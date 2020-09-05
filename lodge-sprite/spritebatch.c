/**
* Spritebatch implementation with asynchronous vertex streaming using glMapBufferRange
*
* Author: Johan Yngman <johan.yngman@gmail.com>
*/

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>

#include "spritebatch.h"
#include "color.h"
#include "lodge_opengl.h"
#include "lodge_renderer.h"
#include "lodge_shader.h"

#define STRIDE 5
#define CURRENT_SPRITE batch->sprite_count * STRIDE * 6

struct spritebatch* spritebatch_create()
{
	struct spritebatch* batch = (struct spritebatch*)malloc(sizeof(struct spritebatch));

	batch->offset_stream = 0;
	batch->offset_draw = 0;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &batch->vbo);
	glGenVertexArrays(1, &batch->vao);
	glBindVertexArray(batch->vao);

	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	glBufferData(GL_ARRAY_BUFFER, SPRITEBATCH_BUFFER_CAPACITY, 0, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * STRIDE, (void *) 0);						// Vertex
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * STRIDE, (void *) (sizeof(GLfloat) * 3));	// Texcoord

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return batch;
}

void spritebatch_destroy(struct spritebatch* batch)
{
	glDeleteBuffers(1, &batch->vbo);
	glDeleteVertexArrays(1, &batch->vao);

	free(batch);
}

void spritebatch_begin(struct spritebatch* batch)
{
	batch->sprite_count = 0;

	// Cycle buffer
	if (batch->offset_stream + SPRITEBATCH_CHUNK >= SPRITEBATCH_BUFFER_CAPACITY)
	{
		glBindVertexArray(batch->vao);

		glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
		glBufferData(GL_ARRAY_BUFFER, SPRITEBATCH_BUFFER_CAPACITY, 0, GL_STREAM_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * STRIDE, (void *) 0);						// Vertex
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * STRIDE, (void *) (sizeof(GLfloat) * 3));	// Texcoord

		glBindVertexArray(0);

		batch->offset_stream = 0;
	}

	// Get GPU memory pointer
	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	batch->gpu_vertices = (GLfloat*)glMapBufferRange(GL_ARRAY_BUFFER, batch->offset_stream, SPRITEBATCH_CHUNK, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

	if (batch->gpu_vertices == 0)
	{
		printf("glMapBufferRange failed\n");
		exit(0);
	}

	batch->offset_draw = batch->offset_stream / SPRITEBATCH_VERTEX_SIZE;
	batch->offset_stream += SPRITEBATCH_CHUNK;
}

void spritebatch_end(struct spritebatch* batch)
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void spritebatch_add(struct spritebatch* batch, vec3 pos, vec2 scale, vec2 tex_pos, vec2 tex_bounds)
{
	// Top-left
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 0 + 0] = -0.5f * scale.v[0] + pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 0 + 1] = 0.5f  * scale.v[1] + pos.v[1];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 0 + 2] = 0.0f + pos.v[2];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 0 + 3] = tex_pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 0 + 4] = tex_pos.v[1];

	// Bottom-left
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 1 + 0] = -0.5f * scale.v[0] + pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 1 + 1] = -0.5f * scale.v[1] + pos.v[1];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 1 + 2] = 0.0f + pos.v[2];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 1 + 3] = tex_pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 1 + 4] = tex_pos.v[1] + tex_bounds.v[1];

	// Top-right
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 2 + 0] = 0.5f * scale.v[0] + pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 2 + 1] = 0.5f * scale.v[1] + pos.v[1];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 2 + 2] = 0.0f + pos.v[2];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 2 + 3] = tex_pos.v[0] + tex_bounds.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 2 + 4] = tex_pos.v[1];

	// Top-right
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 3 + 0] = 0.5f * scale.v[0] + pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 3 + 1] = 0.5f * scale.v[1] + pos.v[1];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 3 + 2] = 0.0f + pos.v[2];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 3 + 3] = tex_pos.v[0] + tex_bounds.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 3 + 4] = tex_pos.v[1];

	// Bottom-left 
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 4 + 0] = -0.5f * scale.v[0] + pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 4 + 1] = -0.5f * scale.v[1] + pos.v[1];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 4 + 2] = 0.0f + pos.v[2];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 4 + 3] = tex_pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 4 + 4] = tex_pos.v[1] + tex_bounds.v[1];

	// Bottom-right
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 5 + 0] = 0.5f  * scale.v[0] + pos.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 5 + 1] = -0.5f * scale.v[1] + pos.v[1];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 5 + 2] = 0.0f + pos.v[2];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 5 + 3] = tex_pos.v[0] + tex_bounds.v[0];
	batch->gpu_vertices[CURRENT_SPRITE + STRIDE * 5 + 4] = tex_pos.v[1] + tex_bounds.v[1];

	batch->sprite_count++;
}

void spritebatch_sort(struct spritebatch* batch, spritebatch_sort_fn sorting_function)
{
	qsort((void*)batch->gpu_vertices, (size_t)batch->sprite_count, sizeof(GLfloat) * 30, sorting_function);
}

void spritebatch_render(struct spritebatch* batch, lodge_shader_t s)
{
	lodge_renderer_bind_shader(s);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	glBindVertexArray(batch->vao);

	/* Position stream. */
	GLint posAttrib = lodge_shader_get_constant_index(s, strview_static("vertex_in"));
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * STRIDE, (void *)0);

	/* Texcoord stream. */
	GLint texcoordAttrib = lodge_shader_get_constant_index(s, strview_static("texcoord_in"));
	glEnableVertexAttribArray(texcoordAttrib);
	glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * STRIDE, (void *)(sizeof(GLfloat) * 3));

	glDrawArrays(GL_TRIANGLES, batch->offset_draw, batch->sprite_count * 6);
}

void spritebatch_render_simple(struct spritebatch* batch, lodge_shader_t s, lodge_texture_t texture, mat4 projection, mat4 transform)
{
	lodge_renderer_bind_shader(s);
	lodge_renderer_bind_texture_2d(0, texture);

	lodge_renderer_set_constant_mat4(s, strview_static("transform"), transform);
	lodge_renderer_set_constant_mat4(s, strview_static("projection"), projection);
	lodge_renderer_set_constant_mat4(s, strview_static("projection"), projection);
	lodge_renderer_set_constant_vec4(s, strview_static("color"), COLOR_WHITE);

#if 0
	GLint uniform_tex = glGetUniformLocation(s->program, "tex");
	glUniform1i(uniform_tex, 0);
#endif

	spritebatch_render(batch, s);
}
