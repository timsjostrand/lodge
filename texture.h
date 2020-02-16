#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <stdint.h>
#include "math4.h"

#define TEXTURE_OK		0
#define TEXTURE_ERROR	-1

typedef unsigned int tex_t;

int  texture_load(tex_t *tex, int *width, int *height, const uint8_t *data,
		size_t len);
int  texture_load_pixels(tex_t *tex, const uint8_t *data,
		const int width, const int height);
void texture_white(tex_t *tex);
int  texture_solid_color(tex_t *tex, int w, int h, const vec4 color);
void texture_free(const tex_t tex);

#endif
