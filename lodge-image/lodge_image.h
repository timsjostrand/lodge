#ifndef _LODGE_IMAGE_H
#define _LODGE_IMAGE_H

#include "lodge.h"

struct lodge_image
{
	uint8_t		*pixel_data;
	int			width;
	int			height;
	int			components;
};

struct blob;

struct lodge_ret	lodge_image_new(struct lodge_image *image_out, const uint8_t *data, size_t len);
struct lodge_ret	lodge_image_new_from_blob(struct lodge_image *image_out, struct blob *blob);
struct lodge_ret	lodge_image_new_solid_color(struct lodge_image *image_out, int width, int height, vec4 color);
void				lodge_image_free(struct lodge_image *image);

#endif
