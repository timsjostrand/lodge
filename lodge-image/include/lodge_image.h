#ifndef _LODGE_IMAGE_H
#define _LODGE_IMAGE_H

#include "lodge.h"

#include <stdint.h>

struct lodge_image_desc
{
	uint32_t					width;
	uint32_t					height;
	uint8_t						channels;
	uint8_t						bytes_per_channel;
};

struct lodge_image
{
	struct lodge_image_desc		desc;
	uint8_t						*pixel_data;

	//
	// HACK(TS): the lodge_image API should be fixed. lodge_image
	// should never own the `pixel_data` memory, but the API
	// implies it (_new(), _free()), which has led to some confusion
	// in the code.
	//
	bool						shared_pixel_data;
};

struct blob;

size_t							lodge_image_desc_get_data_size(const struct lodge_image_desc *desc);
size_t							lodge_image_desc_get_offset_row(const struct lodge_image_desc *desc, uint32_t y);
size_t							lodge_image_desc_get_offset_pixel(const struct lodge_image_desc *desc, uint32_t x, uint32_t y);
size_t							lodge_image_desc_get_offset_pixel_channel(const struct lodge_image_desc *desc, uint32_t x, uint32_t y, uint8_t c);
size_t							lodge_image_desc_get_offset_pixel_r(const struct lodge_image_desc *desc, uint32_t x, uint32_t y);
size_t							lodge_image_desc_get_offset_pixel_g(const struct lodge_image_desc *desc, uint32_t x, uint32_t y);
size_t							lodge_image_desc_get_offset_pixel_b(const struct lodge_image_desc *desc, uint32_t x, uint32_t y);
size_t							lodge_image_desc_get_offset_pixel_a(const struct lodge_image_desc *desc, uint32_t x, uint32_t y);

struct lodge_ret				lodge_image_new(struct lodge_image *image_out, const uint8_t *data, size_t len);
struct lodge_ret				lodge_image_new_from_blob(struct lodge_image *image_out, struct blob *blob);
struct lodge_ret				lodge_image_new_solid_color(struct lodge_image *image_out, uint32_t width, uint32_t height, vec4 color);
void							lodge_image_free(struct lodge_image *image);

void							lodge_image_new_mipmap_inplace(struct lodge_image *dst, const struct lodge_image *src);
void							lodge_image_new_max_mipmap_inplace(struct lodge_image *dst, const struct lodge_image *src);
void							lodge_image_new_min_mipmap_inplace(struct lodge_image *dst, const struct lodge_image *src);

const uint8_t*					lodge_image_get_row(const struct lodge_image *image, uint32_t y);
const uint8_t*					lodge_image_get_pixel(const struct lodge_image *image, uint32_t x, uint32_t y);
const uint8_t*					lodge_image_get_pixel_channel(const struct lodge_image *image, uint32_t x, uint32_t y, uint8_t c);
const uint8_t*					lodge_image_get_pixel_r(const struct lodge_image *image, uint32_t x, uint32_t y);
const uint8_t*					lodge_image_get_pixel_g(const struct lodge_image *image, uint32_t x, uint32_t y);
const uint8_t*					lodge_image_get_pixel_b(const struct lodge_image *image, uint32_t x, uint32_t y);

//
// Given a normalized position (0-1), returns a normalized channel value.
//
float							lodge_image_get_pixel_channel_01(const struct lodge_image *image, vec2 p, uint8_t c);
float							lodge_image_get_pixel_r_01(const struct lodge_image *image, vec2 p);
float							lodge_image_get_pixel_g_01(const struct lodge_image *image, vec2 p);
float							lodge_image_get_pixel_b_01(const struct lodge_image *image, vec2 p);

float							lodge_image_pixel_channel_get_01(const uint8_t *pixel_channel, uint8_t bytes_per_channel);
uint8_t*						lodge_image_pixel_channel_set_01(uint8_t *pixel_channel, uint8_t bytes_per_channel, float value);

#endif
