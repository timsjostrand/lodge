#ifndef _TEXTURE_H
#define _TEXTURE_H

//
// DEPRECATED: see `lodge_texture.h`
//

#include "lodge_texture.h"
#include "math4.h"
#include <stdint.h>

#define TEXTURE_OK		0
#define TEXTURE_ERROR	-1

int  texture_load(lodge_texture_t *tex, int *width, int *height, const uint8_t *data, size_t len);
int  texture_load_pixels(lodge_texture_t *tex, const uint8_t *data, const int width, const int height);
void texture_white(lodge_texture_t *tex);
int  texture_solid_color(lodge_texture_t *tex, int w, int h, const vec4 color);

#endif
