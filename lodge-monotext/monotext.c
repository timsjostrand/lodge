/**
 * Simple monospace font rendering.
 *
 * - monofont_new() parses a new font from a PNG file.
 *
 * - monotext_new() creates a new text object with a specific font. The font
 *   MUST have been completely loaded before monotext_new() is used!
 *
 * - monotext_update() updates the vertex buffer for a monotext with a new text
 *   to display, possibly reallocating memory and pushing new vertices to the
 *   GPU.
 *
 * - monotext_updatef() does the same but takes a printf-style format as well.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include "monotext.h"
#include "lodge.h"
#include "shader.h"
#include "vfs.h"
#include "color.h"
#include "str.h"
#include "lodge_opengl.h"
#include "lodge_renderer.h"
#include "lodge_image.h"

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

/**
 * Looks up a character in the texture atlas and returns the texture coordinates
 * for that character mapped to [0,1].
 */
static int monofont_atlas_coords(float *tx, float *ty, float *tw, float *th,
		struct monofont *font, const char c)
{
	char letter = c - MONOFONT_START_CHAR;

	/* Invalid char. */
	if(letter > c) {
		return MONOTEXT_ERROR;
	}
	/* Font not loaded. */
	if(!font->loaded) {
		monotext_error("Font not loaded: \"%s\"\n", font->name);
		return MONOTEXT_ERROR;
	}

	int grid_y = letter / font->grids_x;
	int grid_x = (letter - (grid_y * font->grids_x)) % font->grids_x;

	/* Map coordinates to [0,1]. */
	*(tx) = grid_x / (float) font->grids_x;
	*(ty) = grid_y / (float) font->grids_y;
	*(tw) = font->letter_width / (float) font->width;
	*(th) = font->letter_height / (float) font->height;

	return MONOTEXT_OK;
}

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

static void monofont_reload(struct vfs *vfs, strview_t filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		monotext_debug("Skipped reload of texture `" STRVIEW_PRINTF_FMT "` (%u bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	struct monofont *dst = (struct monofont *) userdata;

	if(dst == NULL) {
		monotext_error("Invalid userdata argument to monofont_reload()\n");
		return;
	}

	struct lodge_image image;
	struct lodge_ret ret = lodge_image_new(&image, data, size);
	if(!ret.success) {
		monotext_error("Image load failed: `" STRVIEW_PRINTF_FMT "` (%u bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	lodge_texture_t tex = lodge_texture_make_from_image(&image);
	lodge_sampler_t sampler = lodge_sampler_make_properties((struct lodge_sampler_properties) {
		.min_filter = MIN_FILTER_NEAREST,
		.mag_filter = MAG_FILTER_NEAREST,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE,
	});

	dst->texture = tex;
	dst->sampler = sampler;
	dst->loaded = 1;
	dst->width = image.width;
	dst->height = image.height;
	dst->grids_x = image.width / dst->letter_width;
	dst->grids_y = image.height / dst->letter_height;

	lodge_image_free(&image);
}

int monofont_new(struct monofont *font, const char *name,
		int letter_width, int letter_height,
		int letter_spacing_x, int letter_spacing_y,
		struct vfs *vfs)
{
	/* Font metadata. */
	font->name = name;
	font->letter_width = letter_width;
	font->letter_height = letter_height;
	font->letter_spacing_x = letter_spacing_x;
	font->letter_spacing_y = letter_spacing_y;

	/* Load font texture. */
	vfs_register_callback(vfs, strview_make(name, strlen(name)), &monofont_reload, font);

	return MONOTEXT_OK;
}

void monofont_free(struct monofont *font)
{
	font->loaded = 0;
	//lodge_texture_reset(&font->texture);
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
		struct monofont *font, const float blx, const float bly, struct shader *shader)
{
	if(!font->loaded) {
		monotext_error("Font not loaded!\n");
		return;
	}
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
		struct monofont *f = dst->font;

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
					if(monofont_atlas_coords(&tx, &ty, &tw, &th, f, c) != MONOTEXT_OK) {
						monotext_error("Could not get atlas coords for \"%c\"\n", c);
						continue;
					}
					monotext_set_quad(dst->verts, index,
							blx + x * (f->letter_width + f->letter_spacing_x),	// x
							bly + y * (f->letter_height + f->letter_spacing_y),	// y
							blz,												// z
							(float) f->letter_width,							// w
							(float) f->letter_height,							// h
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
		GLint attrib_pos = glGetAttribLocation(dst->shader->program, ATTRIB_NAME_POSITION);
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
		GL_OK_OR_RETURN();

		/* Texcoord stream. */
		GLint attrib_texcoord = glGetAttribLocation(dst->shader->program, ATTRIB_NAME_TEXCOORD);
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
void monotext_render(struct monotext *text, struct shader *s)
{
	if(text == NULL || text->vao == 0) {
		return;
	}

	lodge_renderer_bind_shader(s);

	struct mvp mvp = {
		.model = mat4_identity(),
		.view = mat4_identity(),
		.projection = mat4_ortho(0, 640, 360, 0, -1, 1),
	};

	/* Upload matrices and color. */
	lodge_renderer_set_constant_mvp(s, &mvp);
	lodge_renderer_set_constant_vec4(s, strview_static("color"), text->color);
	lodge_renderer_bind_texture_unit(0, text->font->texture, text->font->sampler);

	/* Bind vertex array. */
	glBindVertexArray(text->vao);
	
	/* Render it! */
	glDrawArrays(GL_TRIANGLES, 0, text->verts_count);
	GL_OK_OR_ASSERT("Failed to render monotext");
}
