#ifndef _MONOTEXT_H_
#define _MONOTEXT_H_

#include <GLFW/glfw3.h>

#include "math4.h"
#include "graphics.h"

#ifdef DEBUG
#define monotext_debug(...) fprintf(stderr, "DEBUG @ Monotext: " __VA_ARGS__)
#else
#define monotext_debug(...) do {} while (0)
#endif

#define monotext_error(...) fprintf(stderr, "ERROR @ Monotext: " __VA_ARGS__)

#define TEXT_OK				0
#define TEXT_ERROR			-1

#define MONOTEXT_STR_MAX	255

#define MONOFONT_START_CHAR	32

/* FIXME: generic way to define new sprite types. */
#define SPRITE_TYPE_TEXT	4

struct monofont {
	int				loaded;
	const char		*name;
	GLuint			texture;
	int				width;
	int				height;
	int				letter_width;
	int				letter_height;
	int				letter_spacing_x;
	int				letter_spacing_y;
	int				grids_x;
	int				grids_y;
};

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
	int				verts_len;					/* Length of the vertex array. */
	int				quads_len;					/* The number of quads required (text_len - newlines). */
	GLuint			vbo;						/* VBO for the vertex array. */
	GLuint			vao;
};

void monotext_new(struct monotext *dst, const char *text, const vec4 color,
		struct monofont *font, const float blx, const float bly);
void monotext_updatef(struct monotext *dst, const char *fmt, ...);
void monotext_update(struct monotext *dst, const char *text);
void monotext_render(struct monotext *text, struct shader *s, struct graphics *g);
void monotext_free(struct monotext *text);

int  monofont_new(struct monofont *font, const char *name,
		int letter_width, int letter_height,
		int letter_spacing_x, int letter_spacing_y);
void monofont_free(struct monofont *font);

#endif
