/**
 * Render drawables (vertex buffer objects) and utility functions to create them
 * from simple geometric primitives.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include "drawable.h"

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <math.h>

#include "lodge_platform.h"
#include "math4.h"
#include "vertex.h"
#include "vertex_buffer.h"
#include "color.h"
#include "lodge_opengl.h"

/* Number of components in a vertex(x, y, z, u, v). */
#define VBO_VERTEX_LEN					5
/* Number of vertices in a quad. */
#define VBO_QUAD_VERTEX_COUNT			6
/* Number of components in a quad. */
#define VBO_QUAD_LEN					(VBO_QUAD_VERTEX_COUNT * VBO_VERTEX_LEN)

static int32_t lodge_gfx_primitive_to_gl(enum lodge_gfx_primitive dm)
{
	switch(dm)
	{
	case LODGE_GFX_PRIMITIVE_POINTS:
		return GL_POINTS;
	case LODGE_GFX_PRIMITIVE_LINE_STRIP:
		return GL_LINE_STRIP;
	case LODGE_GFX_PRIMITIVE_LINE_LOOP:
		return GL_LINE_LOOP;
	case LODGE_GFX_PRIMITIVE_LINES:
		return GL_LINES;
	case LODGE_GFX_PRIMITIVE_LINE_STRIP_ADJACENCY:
		return GL_LINE_STRIP_ADJACENCY;
	case LODGE_GFX_PRIMITIVE_LINES_ADJACENCY:
		return GL_LINES_ADJACENCY;
	case LODGE_GFX_PRIMITIVE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case LODGE_GFX_PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case LODGE_GFX_PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case LODGE_GFX_PRIMITIVE_TRIANGLE_STRIP_ADJACENCY:
		return GL_TRIANGLE_STRIP_ADJACENCY;
	case LODGE_GFX_PRIMITIVE_TRIANGLES_ADJACENCY:
		return GL_TRIANGLES_ADJACENCY;
	case LODGE_GFX_PRIMITIVE_PATCHES:
		return GL_PATCHES;
	default:
		ASSERT_FAIL("Invalid draw_mode");
		return -1;
	}
}

/**
 * Upload vertices to GPU.
 */
static void drawable_set_vbo_xyzuv(GLfloat *vertices, GLuint vertex_count, GLuint *vbo, GLuint *vao)
{
	/* Generate new vertex array? */
	if(*vao == 0) {
		glGenVertexArrays(1, vao);
		GL_OK_OR_RETURN();
	}

	/* Bind vertex array. */
	glBindVertexArray(*vao);
	GL_OK_OR_RETURN();

	/* Generate vertex buffer? */
	if(*vbo == 0) {
		glGenBuffers(1, vbo);
		GL_OK_OR_RETURN();
	}

	/* Bind and upload buffer. */
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	GL_OK_OR_RETURN();
	glBufferData(GL_ARRAY_BUFFER, vertex_count * VBO_VERTEX_LEN * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	GL_OK_OR_RETURN();

	/* Positions */
	{
		static const GLint attrib_pos = 0;
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
		GL_OK_OR_RETURN();
	}

	/* UVs */
	{
		//GLint attrib_texcoord = glGetAttribLocation(s->program, ATTRIB_NAME_TEXCOORD);
		static const GLint attrib_texcoord = 1;
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		GL_OK_OR_RETURN();
	}

	/* Reset bindings to default */
	glBindVertexArray(0);
	GL_OK_OR_RETURN();
}

#if 0
/**
 * Obtain a matrix of vertices (xyzuv) for a circle with the given segments.
 *
 * Inspired by http://slabode.exofire.net/circle_draw.shtml.
 *
 * @param dst		The buffer that will be filled with the vertices for the circle.
 *					Size should be ((segments + 1) * vertex_length), where
 *					vertex_length is 5 (xyzuv).
 * @param cx		Center X coordinate of the circle.
 * @param cy		Center Y coordinate of the circle.
 * @param r			Radius of the circle.
 * @param segments	The number of line segments to create draw this circle from.
 *					Greater number produces a better approximation of a circle
 *					but worsens performance.
 */
static void drawable_get_vertices_circle(GLfloat *dst, float cx, float cy, float r, int segments)
{
	float theta = 2 * (float)M_PI / (float) (segments - 1);
	float c = cosf(theta); // Precalculate.
	float s = sinf(theta);
	float t;

	float x = r; // Start at angle = 0.
	float y = 0;

	for(int i = 0; i < segments; i++) {
		/* Set vertex. */
		dst[i * VBO_VERTEX_LEN + 0] = x + cx;
		dst[i * VBO_VERTEX_LEN + 1] = y + cy;
		dst[i * VBO_VERTEX_LEN + 2] = 0.0f;
		dst[i * VBO_VERTEX_LEN + 3] = 0.0f;
		dst[i * VBO_VERTEX_LEN + 4] = 0.0f;

		/* Apply the rotation matrix. */
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
}
#endif

/**
 * Produces a matrix of vertices (xyzuv) for a rectangle of the given
 * dimensions.
 *
 * @param dst	Destination buffer. Size should be (6 * vertex_length) where
 *				vertex_length is 5 (xyzuv).
 * @param x		Center X coordinate.
 * @param y		Center Y coordinate.
 * @param w		Width.
 * @param h		Height.
 */
static void drawable_get_vertices_rect_solid(GLfloat *dst, float x, float y, float w, float h)
{
	w /= 2.0f;
	h /= 2.0f;

	/* Top-left */
	dst[0 * VBO_VERTEX_LEN + 0] = x - w;	// x
	dst[0 * VBO_VERTEX_LEN + 1] = y + h;	// y
	dst[0 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[0 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[0 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Bottom-Left */
	dst[1 * VBO_VERTEX_LEN + 0] = x - w;	// x
	dst[1 * VBO_VERTEX_LEN + 1] = y - h;	// y
	dst[1 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[1 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[1 * VBO_VERTEX_LEN + 4] = 1.0f;		// v
	/* Top-right */
	dst[2 * VBO_VERTEX_LEN + 0] = x + w;	// x
	dst[2 * VBO_VERTEX_LEN + 1] = y + h;	// y
	dst[2 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[2 * VBO_VERTEX_LEN + 3] = 1.0f;		// u
	dst[2 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Top-right */
	dst[3 * VBO_VERTEX_LEN + 0] = x + w;	// x
	dst[3 * VBO_VERTEX_LEN + 1] = y + h;	// y
	dst[3 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[3 * VBO_VERTEX_LEN + 3] = 1.0f;		// u
	dst[3 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Bottom-left */
	dst[4 * VBO_VERTEX_LEN + 0] = x - w;	// x
	dst[4 * VBO_VERTEX_LEN + 1] = y - h;	// y
	dst[4 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[4 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[4 * VBO_VERTEX_LEN + 4] = 1.0f;		// v
	/* Bottom-right */
	dst[5 * VBO_VERTEX_LEN + 0] = x + w;	// x
	dst[5 * VBO_VERTEX_LEN + 1] = y - h;	// y
	dst[5 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[5 * VBO_VERTEX_LEN + 3] = 1.0f;		// u
	dst[5 * VBO_VERTEX_LEN + 4] = 1.0f;		// v
}

/**
 * Produces a matrix of vertices (xyzuv) for the outline of a rectangle of
 * the given dimensions.
 *
 * @param dst	Destination buffer. Size should be (5 * vertex_length) where
 *				vertex_length is 5 (xyzuv).
 * @param x		Center X coordinate.
 * @param y		Center Y coordinate.
 * @param w		Width.
 * @param h		Height.
 */
static void drawable_get_vertices_rect_outline(GLfloat *dst, float x, float y, float w, float h)
{
	w /= 2.0f;
	h /= 2.0f;

	/* Top-left */
	dst[0 * VBO_VERTEX_LEN + 0] = x - w;	// x
	dst[0 * VBO_VERTEX_LEN + 1] = y + h;	// y
	dst[0 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[0 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[0 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Top-right */
	dst[1 * VBO_VERTEX_LEN + 0] = x + w;	// x
	dst[1 * VBO_VERTEX_LEN + 1] = y + h;	// y
	dst[1 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[1 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[1 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Bottom-right */
	dst[2 * VBO_VERTEX_LEN + 0] = x + w;	// x
	dst[2 * VBO_VERTEX_LEN + 1] = y - h;	// y
	dst[2 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[2 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[2 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Bottom-left */
	dst[3 * VBO_VERTEX_LEN + 0] = x - w;	// x
	dst[3 * VBO_VERTEX_LEN + 1] = y - h;	// y
	dst[3 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[3 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[3 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
	/* Top-left */
	dst[4 * VBO_VERTEX_LEN + 0] = x - w;	// x
	dst[4 * VBO_VERTEX_LEN + 1] = y + h;	// y
	dst[4 * VBO_VERTEX_LEN + 2] = 0.0f;		// z
	dst[4 * VBO_VERTEX_LEN + 3] = 0.0f;		// u
	dst[4 * VBO_VERTEX_LEN + 4] = 0.0f;		// v
}

static struct drawable drawable_make_xyzuv(enum lodge_gfx_primitive primitive, const xyzuv_t *vertices, size_t vertex_count)
{
	ASSERT(vertices);
	ASSERT(vertex_count > 0);

	struct drawable drawable = drawable_make(primitive, vertex_count, 0, 0);

	glGenVertexArrays(1, &drawable.vao);
	GL_OK_OR_RETURN(drawable);

	/* Bind vertex array. */
	glBindVertexArray(drawable.vao);
	GL_OK_OR_RETURN(drawable);

	/* Generate vertex buffer? */
	glGenBuffers(1, &drawable.vbo);
	GL_OK_OR_RETURN(drawable);

	/* Bind and upload buffer. */
	glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
	GL_OK_OR_RETURN(drawable);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(xyzuv_t), vertices, GL_STATIC_DRAW);
	GL_OK_OR_RETURN(drawable);

	/* Positions */
	{
		static const GLint attrib_pos = 0;
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(xyzuv_t), (const void*)offsetof(xyzuv_t, pos));
		GL_OK_OR_RETURN(drawable);
	}

	/* UVs */
	{
		static const GLint attrib_texcoord = 1;
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(xyzuv_t), (const void*)offsetof(xyzuv_t, uv));
		GL_OK_OR_RETURN(drawable);
	}

	/* Reset bindings to default */
	glBindVertexArray(0);
	GL_OK_OR_RETURN(drawable);

	return drawable;
}

static struct drawable drawable_make_vertex(enum lodge_gfx_primitive primitive, const vertex_t *vertices, GLuint vertex_count)
{
	ASSERT(vertices);
	ASSERT(vertex_count > 0);

	struct drawable drawable = drawable_make(primitive, vertex_count, 0, 0);

	/* Generate new vertex array? */
	glGenVertexArrays(1, &drawable.vao);
	GL_OK_OR_RETURN(drawable);

	/* Bind vertex array. */
	glBindVertexArray(drawable.vao);
	GL_OK_OR_RETURN(drawable);

	/* Generate vertex buffer? */
	glGenBuffers(1, &drawable.vbo);
	GL_OK_OR_RETURN(drawable);

	/* Bind and upload buffer. */
	glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
	GL_OK_OR_RETURN(drawable);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex_t), vertices, GL_STATIC_DRAW);
	GL_OK_OR_RETURN(drawable);

	/* Positions */
	{
		static const GLint attrib_pos = 0;
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, pos));
		GL_OK_OR_RETURN(drawable);
	}

	/* UVs */
	{
		static const GLint attrib_texcoord = 1;
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, uv));
		GL_OK_OR_RETURN(drawable);
	}

	/* Tangents */
	{
		static const GLint attrib_tangent = 2;
		glEnableVertexAttribArray(attrib_tangent);
		glVertexAttribPointer(attrib_tangent, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tangent));
		GL_OK_OR_RETURN(drawable);
	}

	/* Bitangents */
	{
		static const GLint attrib_bitangent = 3;
		glEnableVertexAttribArray(attrib_bitangent);
		glVertexAttribPointer(attrib_bitangent, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, bitangent));
		GL_OK_OR_RETURN(drawable);
	}

	glBindVertexArray(0);
	GL_OK_OR_RETURN(drawable);

	return drawable;
}

struct drawable drawable_make(enum lodge_gfx_primitive primitive, GLuint vertex_count, GLuint vbo, GLuint vao)
{
	return (struct drawable) {
		.primitive = primitive,
		.vertex_count = vertex_count,
		.vbo = vbo,
		.vao = vao,
	};
}

#if 0
struct drawable drawable_make_from_buffer(struct vertex_buffer *vb, enum lodge_gfx_primitive primitive)
{
	struct drawable drawable = { 0 };

	drawable.vertex_count = vertex_buffer_get_element_count(vb);

	/* Generate new vertex array? */
	glGenVertexArrays(1, &drawable.vao);
	GL_OK_OR_RETURN(drawable);

	/* Bind vertex array. */
	glBindVertexArray(drawable.vao);
	GL_OK_OR_RETURN(drawable);

	/* Generate vertex buffer? */
	glGenBuffers(1, &drawable.vbo);
	GL_OK_OR_RETURN(drawable);

	/* Bind and upload buffer. */
	glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
	GL_OK_OR_RETURN(drawable);

	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_get_data_size(vb), vertex_buffer_get_data(vb), GL_STATIC_DRAW);
	GL_OK_OR_RETURN(drawable);

	struct vertex_buffer_attribs attribs = vertex_buffer_get_attribs(vb);
	for(int i = 0; i < VERTEX_BUFFER_ATTRIBS_MAX; i++) {
		const struct vertex_buffer_attrib *vertex_attrib = &attribs.per_vertex[i];

		if(vertex_attrib->size > 0) {
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i,
				vertex_attrib->size / sizeof(GLfloat),
				GL_FLOAT,
				GL_FALSE,
				vertex_buffer_get_element_size(vb),
				(const void*)(vertex_attrib->offset)
			);

			GL_OK_OR_RETURN(drawable);
		}
	}

	/* Reset bindings to default */
	glBindVertexArray(0);
	GL_OK_OR_RETURN(drawable);

	return drawable;
}
#endif

void drawable_reset(struct drawable *d)
{
	if(d->vao > 0) {
		glDeleteVertexArrays(1, &d->vao);
	}
	if(d->vbo > 0) {
		glDeleteBuffers(1, &d->vbo);
	}
	*d = drawable_make(0, 0, 0, 0);
}

void drawable_render(const struct drawable *d)
{
	glBindVertexArray(d->vao);
	glDrawArrays(lodge_gfx_primitive_to_gl(d->primitive), 0, d->vertex_count);
}

#if 0
void drawable_render_detailed(enum lodge_gfx_primitive primitive, GLuint vao, GLuint vertex_count, GLuint *tex, vec4 color, lodge_shader_t s, struct mvp mvp)
{
	ASSERT(vertex_count > 0);
	ASSERT(s->program != 0);
	ASSERT(vao != 0);

	/* Bind vertices. */
	glUseProgram(s->program);
	GL_OK_OR_ASSERT("Failed to use shader program");

	glBindVertexArray(vao);
	GL_OK_OR_ASSERT("Failed to bind VAO");

	/* Upload color */
	GLint uniform_color = glGetUniformLocation(s->program, "color");
	if(uniform_color != -1) {
		glUniform4fv(uniform_color, 1, (GLfloat*)color.v);
		GL_OK_OR_ASSERT("Could not set uniform `color`");
	}

	/* Upload transform */
	GLint uniform_projection = glGetUniformLocation(s->program, "projection");
	if(uniform_projection != -1) {
		glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, (GLfloat*)mvp.projection.m);
		GL_OK_OR_ASSERT("Could not set uniform `projection`");
	}

	/* Upload transform */
	GLint uniform_view = glGetUniformLocation(s->program, "view");
	if(uniform_view != -1) {
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, (GLfloat*)mvp.view.m);
		GL_OK_OR_ASSERT("Could not set uniform `view`");
	}

	/* Upload transform */
	GLint uniform_model = glGetUniformLocation(s->program, "model");
	if(uniform_model != -1) {
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, (GLfloat*)mvp.model.m);
		GL_OK_OR_ASSERT("Could not set uniform `model`");
	}

	/* Render it! */
	if(tex) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (*tex));
		GL_OK_OR_ASSERT("Could not bind drawable texture");
	}

	glDrawArrays(lodge_gfx_primitive_to_gl(primitive), 0, vertex_count);
	GL_OK_OR_RETURN();

	glBindVertexArray(0);
	GL_OK_OR_RETURN();
}

void drawable_render_simple(struct drawable *d, lodge_shader_t s, GLuint *tex, vec4 color, mat4 view)
{
	struct mvp mvp = {
		.model = mat4_identity(),
		.view = view,
		.projection = mat4_identity()
	};
	drawable_render_detailed(d->primitive, d->vao, d->vertex_count, tex, color, s, mvp);
}
#endif

void drawable_new_rect_outline(struct drawable *dst, struct rect *rect)
{
	drawable_new_rect_outlinef(dst, rect->pos.x, rect->pos.y, rect->size.x, rect->size.y);
}

void drawable_new_rect_outlinef(struct drawable *dst, float x, float y, float w, float h)
{
	dst->primitive = GL_LINE_STRIP;
	/* Allocate memory for vertices. */
	dst->vertex_count = 5;
	GLfloat *vertices = (GLfloat *) calloc(dst->vertex_count * VBO_VERTEX_LEN, sizeof(GLfloat));

	/* OOM? */
	if(vertices == NULL) {
		errorf("Drawable", "Out of memory\n");
		return;
	}

	/* Calculate vertices. */
	drawable_get_vertices_rect_outline(vertices, x, y, w, h);

	/* Upload vertices to GPU. */
	drawable_set_vbo_xyzuv(vertices, dst->vertex_count, &dst->vbo, &dst->vao);

	/* Cleanup. */
	free(vertices);
}

void drawable_new_rect_solidf(struct drawable *dst, float x, float y, float w, float h)
{
	dst->primitive = LODGE_GFX_PRIMITIVE_TRIANGLES;
	/* Allocate memory for vertices. */
	dst->vertex_count = 6;
	GLfloat *vertices = (GLfloat *) calloc(dst->vertex_count * VBO_VERTEX_LEN, sizeof(GLfloat));

	/* OOM? */
	if(vertices == NULL) {
		errorf("Drawable", "Out of memory\n");
		return;
	}

	/* Calculate vertices. */
	drawable_get_vertices_rect_solid(vertices, x, y, w, h);

	/* Upload vertices to GPU. */
	drawable_set_vbo_xyzuv(vertices, dst->vertex_count, &dst->vbo, &dst->vao);

	/* Cleanup. */
	free(vertices);
}

static xyzuv_t* drawable_get_vertices_plane_quad(xyzuv_t* dst, float x, float y, float w, float h, int mirror)
{
	if(mirror) {
		dst[0] = xyzuv_make(x,		y + h,	0.0f, 0.0f, 0.0f);	/* Top-left */
		dst[1] = xyzuv_make(x,		y,		0.0f, 0.0f, 1.0f);	/* Bottom-Left */
		dst[2] = xyzuv_make(x + w,	y,		0.0f, 1.0f, 1.0f);	/* Bottom-right */
		dst[3] = xyzuv_make(x + w,	y + h,	0.0f, 1.0f, 0.0f);	/* Top-right */
		dst[4] = xyzuv_make(x,		y + h,	0.0f, 0.0f, 0.0f);	/* Top-left */
		dst[5] = xyzuv_make(x + w,	y,		0.0f, 1.0f, 1.0f);	/* Bottom-right */
	} else {
		dst[0] = xyzuv_make(x,		y + h,	0.0f, 0.0f, 0.0f);	/* Top-left */
		dst[1] = xyzuv_make(x,		y,		0.0f, 0.0f, 1.0f);	/* Bottom-Left */
		dst[2] = xyzuv_make(x + w,	y + h,	0.0f, 1.0f, 0.0f);	/* Top-right */
		dst[3] = xyzuv_make(x + w,	y + h,	0.0f, 1.0f, 0.0f);	/* Top-right */
		dst[4] = xyzuv_make(x,		y,		0.0f, 0.0f, 1.0f);	/* Bottom-left */
		dst[5] = xyzuv_make(x + w,	y,		0.0f, 1.0f, 1.0f);	/* Bottom-right */
	}

	return dst + 6 * sizeof(xyzuv_t);
}

static vertex_t* drawable_get_vertices_plane_quad_vertex(vertex_t* dst, float x, float y, float w, float h, int mirror)
{
	if(mirror) {
		/* Top-left */
		dst[0].x = x;	
		dst[0].y = y + h;
		dst[0].z = 0.0f;
		dst[0].u = x;
		dst[0].v = y + h;
		/* Bottom-Left */
		dst[1].x = x;
		dst[1].y = y;
		dst[1].z = 0.0f;
		dst[1].u = x;	
		dst[1].v = y;	
		/* Bottom-right */
		dst[2].x = x + w;
		dst[2].y = y;
		dst[2].z = 0.0f;
		dst[2].u = x + w;
		dst[2].v = y;
		vertex_calc_tangents(&dst[0], &dst[1], &dst[2]);

		/* Top-right */
		dst[3].x = x + w;
		dst[3].y = y + h;
		dst[3].z = 0.0f;	
		dst[3].u = x + w;
		dst[3].v = y + h;	
		/* Top-left */
		dst[4].x = x;
		dst[4].y = y + h;
		dst[4].z = 0.0f;
		dst[4].u = x;
		dst[4].v = y + h;	
		/* Bottom-right */
		dst[5].x = x + w;
		dst[5].y = y;
		dst[5].z = 0.0f;
		dst[5].u = x + w;
		dst[5].v = y;	
		vertex_calc_tangents(&dst[3], &dst[4], &dst[5]);
	} else {
		/* Top-left */
		dst[0].x = x;
		dst[0].y = y + h;
		dst[0].z = 0.0f;	
		dst[0].u = x;
		dst[0].v = y + h;
		/* Bottom-Left */
		dst[1].x = x;
		dst[1].y = y;
		dst[1].z = 0.0f;	
		dst[1].u = x;
		dst[1].v = y;
		/* Top-right */
		dst[2].x = x + w;
		dst[2].y = y + h;
		dst[2].z = 0.0f;
		dst[2].u = x + w;	
		dst[2].v = y + h;
		vertex_calc_tangents(&dst[0], &dst[1], &dst[2]);

		/* Top-right */
		dst[3].x = x + w;
		dst[3].y = y + h;
		dst[3].z = 0.0f;
		dst[3].u = x + w;	
		dst[3].v = y + h;	
		/* Bottom-left */
		dst[4].x = x;
		dst[4].y = y;
		dst[4].z = 0.0f;	
		dst[4].u = x;
		dst[4].v = y;
		/* Bottom-right */
		dst[5].x = x + w;
		dst[5].y = y;
		dst[5].z = 0.0f;
		dst[5].u = x + w;
		dst[5].v = y;
		vertex_calc_tangents(&dst[3], &dst[4], &dst[5]);
	}

	return dst + 6;
}

struct drawable drawable_make_plane_subdivided(vec2 origin, vec2 size, int divisions_x, int divisions_y)
{
	size_t vertex_count = 6 * divisions_x * divisions_y;
	xyzuv_t *vertices = (xyzuv_t *)calloc(vertex_count, sizeof(xyzuv_t));
	ASSERT(vertices);

	/* Calculate vertices. */
	const float w = size.x / divisions_x;
	const float h = size.y / divisions_y;
	xyzuv_t *cursor = vertices;
	for(int y = 0; y < divisions_y; y++) {
		for(int x = 0; x < divisions_x; x++) {
			float x0 = origin.x + x * w;
			float y0 = origin.y + y * h;
			int mirror = (y + x) % 2 == 0;

			cursor = drawable_get_vertices_plane_quad(cursor, x0, y0, w, h, mirror);
		}
	}

	/* Upload vertices to GPU. */
	struct drawable drawable = drawable_make_xyzuv(LODGE_GFX_PRIMITIVE_TRIANGLES, vertices, vertex_count);

	/* Cleanup. */
	free(vertices);

	return drawable;
}

struct drawable drawable_make_plane_subdivided_vertex(vec2 origin, vec2 size, int divisions_x, int divisions_y)
{
	size_t vertex_count = 6 * divisions_x * divisions_y;
	vertex_t *vertices = (vertex_t *)calloc(vertex_count, sizeof(vertex_t));
	ASSERT(vertices);

	/* Calculate vertices. */
	const float w = size.x / divisions_x;
	const float h = size.y / divisions_y;
	vertex_t *cursor = vertices;
	for(int y = 0; y < divisions_y; y++) {
		for(int x = 0; x < divisions_x; x++) {
			float x0 = origin.x + x * w;
			float y0 = origin.y + y * h;
			int mirror = (y + x) % 2 == 0;

			cursor = drawable_get_vertices_plane_quad_vertex(cursor, x0, y0, w, h, mirror);
		}
	}

	/* Upload vertices to GPU. */
	struct drawable drawable = drawable_make_vertex(LODGE_GFX_PRIMITIVE_TRIANGLES, vertices, vertex_count);

	/* Cleanup. */
	free(vertices);

	return drawable;
}

#if 0
void drawable_new_rect_fullscreen(struct drawable *dst, lodge_shader_t s)
{
	dst->primitive = LODGE_GFX_PRIMITIVE_TRIANGLES;
	/* Allocate memory for vertices. */
	dst->vertex_count = 6;
	GLfloat *vertices = (GLfloat *)calloc(dst->vertex_count * VBO_VERTEX_LEN, sizeof(GLfloat));

	/* OOM? */
	if (vertices == NULL) {
		errorf("Drawable", "Out of memory\n");
		return;
	}

	/* Top-left */
	vertices[0 * VBO_VERTEX_LEN + 0] = -1.0f;	// x
	vertices[0 * VBO_VERTEX_LEN + 1] = 1.0f;	// y
	vertices[0 * VBO_VERTEX_LEN + 2] = 0.0f;	// z
	vertices[0 * VBO_VERTEX_LEN + 3] = 0.0f;	// u
	vertices[0 * VBO_VERTEX_LEN + 4] = 1.0f;	// v

	/* Bottom-right */
	vertices[1 * VBO_VERTEX_LEN + 0] = 1.0f;	// x
	vertices[1 * VBO_VERTEX_LEN + 1] = -1.0f;	// y
	vertices[1 * VBO_VERTEX_LEN + 2] = 0.0f;	// z
	vertices[1 * VBO_VERTEX_LEN + 3] = 1.0f;	// u
	vertices[1 * VBO_VERTEX_LEN + 4] = 0.0f;	// v

	/* Top-right */
	vertices[2 * VBO_VERTEX_LEN + 0] = 1.0f;	// x
	vertices[2 * VBO_VERTEX_LEN + 1] = 1.0f;	// y
	vertices[2 * VBO_VERTEX_LEN + 2] = 0.0f;	// z
	vertices[2 * VBO_VERTEX_LEN + 3] = 1.0f;	// u
	vertices[2 * VBO_VERTEX_LEN + 4] = 1.0f;	// v

	/* Top-left */
	vertices[3 * VBO_VERTEX_LEN + 0] = -1.0f;	// x
	vertices[3 * VBO_VERTEX_LEN + 1] = 1.0f;	// y
	vertices[3 * VBO_VERTEX_LEN + 2] = 0.0f;	// z
	vertices[3 * VBO_VERTEX_LEN + 3] = 0.0f;	// u
	vertices[3 * VBO_VERTEX_LEN + 4] = 1.0f;	// v

	/* Bottom-left */
	vertices[4 * VBO_VERTEX_LEN + 0] = -1.0f;	// x
	vertices[4 * VBO_VERTEX_LEN + 1] = -1.0f;	// y
	vertices[4 * VBO_VERTEX_LEN + 2] = 0.0f;	// z
	vertices[4 * VBO_VERTEX_LEN + 3] = 0.0f;	// u
	vertices[4 * VBO_VERTEX_LEN + 4] = 0.0f;	// v

	/* Bottom-right */
	vertices[5 * VBO_VERTEX_LEN + 0] = 1.0f;	// x
	vertices[5 * VBO_VERTEX_LEN + 1] = -1.0f;	// y
	vertices[5 * VBO_VERTEX_LEN + 2] = 0.0f;	// z
	vertices[5 * VBO_VERTEX_LEN + 3] = 1.0f;	// u
	vertices[5 * VBO_VERTEX_LEN + 4] = 0.0f;	// v

	/* Upload vertices to GPU. */
	drawable_set_vbo_xyzuv(vertices, dst->vertex_count, &dst->vbo, &dst->vao);

	/* Cleanup. */
	free(vertices);
}
#endif

#if 0
void drawable_new_circle_outlinef(struct drawable *dst, float x, float y, float r, int segments)
{
	dst->primitive = GL_LINE_STRIP;
	/* The actual required vertex count is segments+1 because the final vertex
	 * needs to reconnect with the first one. */
	dst->vertex_count = segments + 1;

	/* Allocate memory for vertices. */
	GLfloat *vertices = (GLfloat *) calloc(dst->vertex_count * VBO_VERTEX_LEN, sizeof(GLfloat));

	/* OOM? */
	if(vertices == NULL) {
		errorf("Drawable", "Out of memory\n");
		return;
	}

	/* Calculate vertices. */
	drawable_get_vertices_circle(vertices, x, y, r, dst->vertex_count);

	/* Upload vertices to GPU. */
	drawable_set_vbo_xyzuv(vertices, dst->vertex_count, &dst->vbo, &dst->vao);

	/* Cleanup. */
	free(vertices);
}

/**
 * Creates a vertex buffer for a circle.
 */
void drawable_new_circle_outline(struct drawable *dst, struct circle *circle, int segments)
{
	drawable_new_circle_outlinef(dst, circle->pos.x, circle->pos.y, circle->r, segments);
}
#endif

#if 0
struct drawable drawable_make_line(vec3 start, vec3 end)
{
	const xyzuv_t vertices[] = {
		{ start.x,	start.y,	start.z,	0.0f, 0.0f },
		{ end.x,	end.y,		end.z,		0.0f, 0.0f },
	};
	return drawable_make_xyzuv(GL_LINE_STRIP, vertices, LODGE_ARRAYSIZE(vertices));
}

/**
 * Create a unit cube from (-0.5,-0.5,-0.5) to (0.5,0.5,0.5).
 */
struct drawable drawable_make_unit_cube()
{
	static const xyzuv_t vertices[] = {
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
	};

	return drawable_make_xyzuv(LODGE_GFX_PRIMITIVE_TRIANGLES, vertices, LODGE_ARRAYSIZE(vertices));
}
#endif