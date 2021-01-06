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

#ifndef _MONOTEXT_H_
#define _MONOTEXT_H_

#include "math4.h"

#define MONOTEXT_STR_MAX 1024*10

typedef struct lodge_sampler* lodge_sampler_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct monotext {
	vec3			bottom_left;				/* Bottom-Left origin coordinate. */
	char			text[MONOTEXT_STR_MAX];		/* The text to render. */
	int				text_len;					/* The number of characters in the text. */
	struct monofont *font;						/* The font to render with. */
	vec4			color;
	int				width_chars;				/* The width of the text in characters. */
	int				height_chars;				/* The height of the text in characters. */
	int				width;						/* The width of the text in pixels. */
	int				height;						/* The height fo the text in pixels. */
	float			*verts;						/* The vertices required to draw this text. */
	int				verts_count;				/* Number of vertices in verts buffer. */
	int				verts_len;					/* Length of the vertex buffer. */
	int				quads_count;				/* The number of quads required (text_len - newlines). */
	unsigned int	vbo;						/* VBO for the vertex array. */
	unsigned int	vao;
	lodge_shader_t	shader;
};

void monotext_new(struct monotext *dst, const char *text, const vec4 color, struct monofont *font, const float blx, const float bly, lodge_shader_t shader);
void monotext_updatef(struct monotext *dst, const char *fmt, ...);
void monotext_update(struct monotext *dst, const char *text, const size_t len);
void monotext_render(struct monotext *text, lodge_shader_t s, lodge_sampler_t font_sampler);
void monotext_free(struct monotext *text);

#endif
