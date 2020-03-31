#ifndef _LODGE_TEXTURE_H
#define _LODGE_TEXTURE_H

#include "lodge_platform.h"

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_image;

lodge_texture_t		lodge_texture_make();
lodge_texture_t		lodge_texture_make_rgba(int width, int height);
lodge_texture_t		lodge_texture_make_depth(int width, int height);
lodge_texture_t		lodge_texture_make_from_image(const struct lodge_image *image);

void				lodge_texture_reset(lodge_texture_t *texture);

int					lodge_texture_is_valid(lodge_texture_t texture);

#endif