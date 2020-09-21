#include "monotext.h"

#include "monofont.h"
#include "color.h"
#include "str.h"
#include "log.h"
#include "lodge_opengl.h"
#include "lodge_renderer.h"
#include "lodge_image.h"
#include "lodge_shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <GL/glew.h>

/* Number of components in a vertex(x, y, z, u, v). */
#define VBO_VERTEX_LEN                  5
/* Number of vertices in a quad. */
#define VBO_QUAD_VERTEX_COUNT			6
/* Number of components in a quad. */
#define VBO_QUAD_LEN                    (VBO_QUAD_VERTEX_COUNT * VBO_VERTEX_LEN)

#define monotext_debug(...) debugf("Monotext", __VA_ARGS__)
#define monotext_error(...) errorf("Monotext", __VA_ARGS__)

/**
 * Calculates the bounding box required around a string of text, taking into
 * account new lines as well.
 */
static void strnbounds(const char *s, const int len, int *width_max, int *height_max)
{
	*(width_max) = 0;
	*(height_max) = 0;
	for(int i=0; i<len; i++) {
		switch(s[i]) {
		case '\n':
			(*height_max) ++;
			break;
		case '\0':
			return;
		default:
			(*width_max) ++;
			break;
		}
	}
}

/**
 * @param verts
 * @param index
 * @param x			Quad X component.
 * @param y			Quad Y component.
 * @param z			Quad Z component.
 * @param w			Quad width.
 * @param h			Quad height.
 * @param tx		Texture X offset.
 * @param ty		Texture Y offset.
 * @param tw		Texture width.
 * @param th		Texture height.
 */
static void monotext_set_quad(float *verts, const int quad,
		float x, float y, float z, float w, float h,
		float tx, float ty, float tw, float th)
{
	w /= 2.0f;
	h /= 2.0f;

	/* Row 0: Top-Left? */
	verts[quad * VBO_QUAD_LEN + 0 * VBO_VERTEX_LEN + 0] = x - w;
	verts[quad * VBO_QUAD_LEN + 0 * VBO_VERTEX_LEN + 1] = y + h;
	verts[quad * VBO_QUAD_LEN + 0 * VBO_VERTEX_LEN + 2] = z;
	verts[quad * VBO_QUAD_LEN + 0 * VBO_VERTEX_LEN + 3] = tx;
	verts[quad * VBO_QUAD_LEN + 0 * VBO_VERTEX_LEN + 4] = ty;

	/* Row 1: Bottom-Left? */
	verts[quad * VBO_QUAD_LEN + 1 * VBO_VERTEX_LEN + 0] = x - w;
	verts[quad * VBO_QUAD_LEN + 1 * VBO_VERTEX_LEN + 1] = y - h;
	verts[quad * VBO_QUAD_LEN + 1 * VBO_VERTEX_LEN + 2] = z;
	verts[quad * VBO_QUAD_LEN + 1 * VBO_VERTEX_LEN + 3] = tx;
	verts[quad * VBO_QUAD_LEN + 1 * VBO_VERTEX_LEN + 4] = ty + th;

	/* Row 2: Top-Right? */
	verts[quad * VBO_QUAD_LEN + 2 * VBO_VERTEX_LEN + 0] = x + w;
	verts[quad * VBO_QUAD_LEN + 2 * VBO_VERTEX_LEN + 1] = y + h;
	verts[quad * VBO_QUAD_LEN + 2 * VBO_VERTEX_LEN + 2] = z;
	verts[quad * VBO_QUAD_LEN + 2 * VBO_VERTEX_LEN + 3] = tx + tw;
	verts[quad * VBO_QUAD_LEN + 2 * VBO_VERTEX_LEN + 4] = ty;

	/* Row 3: Top-Right? */
	verts[quad * VBO_QUAD_LEN + 3 * VBO_VERTEX_LEN + 0] = x + w;
	verts[quad * VBO_QUAD_LEN + 3 * VBO_VERTEX_LEN + 1] = y + h;
	verts[quad * VBO_QUAD_LEN + 3 * VBO_VERTEX_LEN + 2] = z;
	verts[quad * VBO_QUAD_LEN + 3 * VBO_VERTEX_LEN + 3] = tx + tw;
	verts[quad * VBO_QUAD_LEN + 3 * VBO_VERTEX_LEN + 4] = ty;

	/* Row 4: Bottom-Left? */
	verts[quad * VBO_QUAD_LEN + 4 * VBO_VERTEX_LEN + 0] = x - w;
	verts[quad * VBO_QUAD_LEN + 4 * VBO_VERTEX_LEN + 1] = y - h;
	verts[quad * VBO_QUAD_LEN + 4 * VBO_VERTEX_LEN + 2] = z;
	verts[quad * VBO_QUAD_LEN + 4 * VBO_VERTEX_LEN + 3] = tx;
	verts[quad * VBO_QUAD_LEN + 4 * VBO_VERTEX_LEN + 4] = ty + th;

	/* Row 5: Bottom-Right? */
	verts[quad * VBO_QUAD_LEN + 5 * VBO_VERTEX_LEN + 0] = x + w;
	verts[quad * VBO_QUAD_LEN + 5 * VBO_VERTEX_LEN + 1] = y - h;
	verts[quad * VBO_QUAD_LEN + 5 * VBO_VERTEX_LEN + 2] = z;
	verts[quad * VBO_QUAD_LEN + 5 * VBO_VERTEX_LEN + 3] = tx + tw;
	verts[quad * VBO_QUAD_LEN + 5 * VBO_VERTEX_LEN + 4] = ty + th;
}

static void monotext_print_vert(float *verts, int quad, int vert)
{
	printf("\tx: %8f\n", verts[quad * VBO_QUAD_LEN + vert * VBO_VERTEX_LEN + 0]);
	printf("\ty: %8f\n", verts[quad * VBO_QUAD_LEN + vert * VBO_VERTEX_LEN + 1]);
	printf("\tz: %8f\n", verts[quad * VBO_QUAD_LEN + vert * VBO_VERTEX_LEN + 2]);
	printf("\tu: %8f\n", verts[quad * VBO_QUAD_LEN + vert * VBO_VERTEX_LEN + 3]);
	printf("\tv: %8f\n", verts[quad * VBO_QUAD_LEN + vert * VBO_VERTEX_LEN + 4]);
}

static void monotext_print_quad(float *verts, int quad)
{
	printf("Top-Left:\n");
	monotext_print_vert(verts, quad, 0); /* Top-Left? */
	printf("Bottom-Left:\n");
	monotext_print_vert(verts, quad, 1); /* Bottom-Left? */
	printf("Top-Right:\n");
	monotext_print_vert(verts, quad, 2); /* Top-Right? */
	printf("Top-Right:\n");
	monotext_print_vert(verts, quad, 3); /* Top-Right? */
	printf("Bottom-Left:\n");
	monotext_print_vert(verts, quad, 4); /* Bottom-Left? */
	printf("Bottom-Right:\n");
	monotext_print_vert(verts, quad, 5); /* Bottom-Right? */
}

/**
 * @param dst	Where to store the new monotext.
 * @param text	The text to draw.
 * @param blx	The bottom left X origin.
 * @param bly	The bottom left Y origin.
 * @param font	The font to render with.
 */
void monotext_new(struct monotext *dst, const char *text, const vec4 color,
		struct monofont *font, const float blx, const float bly, lodge_shader_t shader)
{
	dst->font = font;
	dst->shader = shader;
	dst->bottom_left = vec3_make(blx, bly, 0.2f);
	dst->color = color;
	monotext_update(dst, text, strnlen(text, MONOTEXT_STR_MAX));
}

void monotext_free(struct monotext *text)
{
	free(text->verts);
	glDeleteVertexArrays(1, &text->vao);
	glDeleteBuffers(1, &text->vbo);
}

/**
 * FIXME: can probably live without intermediate buffer, write directly to
 * dst->text? Will cause problems with detecting if dst->text changed. Introduce
 * force update flag?
 */
void monotext_updatef(struct monotext *dst, const char *fmt, ...)
{
	char s[MONOTEXT_STR_MAX] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf(s, fmt, args);
	va_end(args);

	monotext_update(dst, s, strnlen(s, MONOTEXT_STR_MAX));
}

/**
 * Updates the displayed text of a monotext, potentially reallocating memory and
 * pushing new buffer objects to the GPU.
 */
void monotext_update(struct monotext *dst, const char *text, const size_t len)
{
	/* Sanity check. */
	if(len >= MONOTEXT_STR_MAX) {
		monotext_error("text length %d >= MONOTEXT_STR_MAX!\n", (int) len);
		return;
	}

	/* Set this flag if more/less vert memory is required for vertices. */
	int realloc_verts = 0;
	/* Set this flag if vertices are to be re-uploaded to the GPU. */
	int rearrange_verts = (dst->text == NULL) || !(strcmp(dst->text, text) == 0);

	/* Text metadata. */
	strncpy(dst->text, text, MONOTEXT_STR_MAX);
	dst->text_len = strnlen(text, MONOTEXT_STR_MAX);

	/* Text bounds. */
	strnbounds(dst->text, dst->text_len, &(dst->width_chars), &(dst->height_chars));
	dst->width = dst->width_chars * dst->font->letter_width;
	dst->height = dst->height_chars * dst->font->letter_height;

	/* Create vertices: 5 rows of floats for every letter. */
	int old_verts_len = dst->verts_len;
	dst->quads_count = dst->text_len - dst->height_chars;
	dst->verts_count = dst->quads_count * VBO_QUAD_VERTEX_COUNT;
	dst->verts_len = dst->quads_count * VBO_QUAD_LEN;
	realloc_verts = old_verts_len != dst->verts_len;

	/* Reallocate vertices memory if necessary. */
	if(realloc_verts) {
		free(dst->verts);
		dst->verts = (float *) calloc(dst->verts_len * VBO_VERTEX_LEN, sizeof(float));

		/* Create vertex buffer. */
		if(glIsBuffer(dst->vbo) == GL_FALSE) {
			glGenBuffers(1, &dst->vbo);
		}

		/* Create vertex array. */
		if(glIsVertexArray(dst->vao) == GL_FALSE) {
			glGenVertexArrays(1, &dst->vao);
		}
	}

	/* Update letter sprites if necessary. */
	if(rearrange_verts) {
		int index = 0;
		int x = 0;
		int y = dst->height_chars;
		float blx = dst->bottom_left.v[0];
		float bly = dst->bottom_left.v[1];
		float blz = dst->bottom_left.v[2];
		struct monofont *font = dst->font;

		for(int i=0; i < dst->text_len; i++) {
			char c = dst->text[i];
			/* Handle new lines. */
			switch(c) {
			case '\0':
				break;
			case '\n':
				x = 0;
				y--;
				break;
			default: {
					float tx, ty, tw, th;
					if(!monofont_get_atlas_quad_01(font, c, &tx, &ty, &tw, &th)) {
						monotext_error("Could not get atlas coords for \"%c\"\n", c);
						continue;
					}
					monotext_set_quad(dst->verts, index,
							blx + x * (font->letter_width + font->letter_spacing_x),	// x
							bly + y * (font->letter_height + font->letter_spacing_y),	// y
							blz,												// z
							(float) font->letter_width,							// w
							(float) font->letter_height,							// h
							tx, ty, tw, th);
					index++;
					x++;
					break;
				}
			}
		}

		/* Bind vertex array. */
		glBindVertexArray(dst->vao);
		GL_OK_OR_RETURN();

		/* glBufferData reallocates memory if necessary. */
		glBindBuffer(GL_ARRAY_BUFFER, dst->vbo);
		glBufferData(GL_ARRAY_BUFFER, dst->verts_len * sizeof(GLfloat),
				dst->verts, GL_DYNAMIC_DRAW);
		GL_OK_OR_RETURN();

		/* Position stream. */
		const int attrib_pos = lodge_shader_get_constant_index(dst->shader, strview_static("vertex_in"));
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
		GL_OK_OR_RETURN();

		/* Texcoord stream. */
		const int attrib_texcoord = lodge_shader_get_constant_index(dst->shader, strview_static("texcoord_in"));
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
				(void*) (3 * sizeof(GLfloat)));
		GL_OK_OR_RETURN();
	}
}

/**
 * FIXME: optimize:
 * - use DrawElements instead of DrawArrays (requires indices array).
 */
void monotext_render(struct monotext *text, lodge_shader_t s, lodge_sampler_t atlas_sampler)
{
	if(text == NULL || text->vao == 0) {
		return;
	}

	lodge_renderer_bind_shader(s);

	struct mvp mvp = {
		.model = mat4_identity(),
		.view = mat4_identity(),
		.projection = mat4_ortho(0, 640, 0, 360, -1, 1),
	};

	/* Upload matrices and color. */
	lodge_renderer_set_constant_mvp(s, &mvp);
	lodge_renderer_set_constant_vec4(s, strview_static("color"), text->color);
	lodge_renderer_bind_texture_unit_2d(0, text->font->atlas_texture, atlas_sampler);

	/* Bind vertex array. */
	glBindVertexArray(text->vao);
	
	/* Render it! */
	glDrawArrays(GL_TRIANGLES, 0, text->verts_count);
	GL_OK_OR_ASSERT("Failed to render monotext");
}
