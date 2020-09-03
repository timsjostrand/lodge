#ifndef _LODGE_TEXTURE_H
#define _LODGE_TEXTURE_H

#include "lodge_platform.h"

enum lodge_texture_target
{
	LODGE_TEXTURE_TARGET_2D,
	LODGE_TEXTURE_TARGET_CUBE_MAP,
};

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_texture_cubemap_desc
{
	const struct lodge_image *front;
	const struct lodge_image *back;
	const struct lodge_image *top;
	const struct lodge_image *bottom;
	const struct lodge_image *left;
	const struct lodge_image *right;
};

struct lodge_image;

lodge_texture_t		lodge_texture_make();
lodge_texture_t		lodge_texture_make_rgba(uint32_t width, uint32_t height);
lodge_texture_t		lodge_texture_make_depth(uint32_t width, uint32_t height);
lodge_texture_t		lodge_texture_make_from_image(const struct lodge_image *image);
lodge_texture_t		lodge_texture_make_cubemap(struct lodge_texture_cubemap_desc desc);

void				lodge_texture_reset(lodge_texture_t *texture);

int					lodge_texture_is_valid(lodge_texture_t texture);

#endif