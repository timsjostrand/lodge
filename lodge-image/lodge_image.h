#ifndef _LODGE_IMAGE_H
#define _LODGE_IMAGE_H

#include "lodge.h"

struct lodge_image_out
{
	uint8_t		*pixel_data;
	int			width;
	int			height;
	int			components;
};

struct blob;

struct lodge_ret	lodge_image_new(struct lodge_image_out *out, const uint8_t *data, size_t len);
struct lodge_ret	lodge_image_new_from_blob(struct lodge_image_out *out, struct blob *blob);
void				lodge_image_free(uint8_t *data);

#endif
