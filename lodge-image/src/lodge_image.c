#include "lodge_image.h"

#include "blob.h"
#include <stb/stb_image.h>

#define COLOR_TO_UINT8(COLOR) \
	{ \
		(uint8_t)(COLOR.r * 255.0f), \
		(uint8_t)(COLOR.g * 255.0f), \
		(uint8_t)(COLOR.b * 255.0f), \
		(uint8_t)(COLOR.a * 255.0f), \
	};

size_t lodge_image_desc_get_data_size(const struct lodge_image_desc *desc)
{
	ASSERT(desc);
	return (size_t)desc->width * (size_t)desc->height * (size_t)desc->channels * (size_t)desc->bytes_per_channel;
}

size_t lodge_image_desc_get_offset_row(const struct lodge_image_desc *desc, uint32_t y)
{
	ASSERT(desc);
	ASSERT(y < desc->height);
	const size_t stride = (size_t)desc->width * (size_t)desc->channels * (size_t)desc->bytes_per_channel;
	return y * stride;
}

size_t lodge_image_desc_get_offset_pixel(const struct lodge_image_desc *desc, uint32_t x, uint32_t y)
{
	ASSERT(desc);
	ASSERT(x < desc->width);
	return lodge_image_desc_get_offset_row(desc, y) + x * (size_t)desc->channels * (size_t)desc->bytes_per_channel;
}

size_t lodge_image_desc_get_offset_pixel_channel(const struct lodge_image_desc *desc, uint32_t x, uint32_t y, uint8_t c)
{
	ASSERT(desc);
	ASSERT(c < desc->channels);
	return lodge_image_desc_get_offset_pixel(desc, x, y) + (size_t)c * (size_t)desc->bytes_per_channel;
}

size_t lodge_image_desc_get_offset_pixel_r(const struct lodge_image_desc *desc, uint32_t x, uint32_t y)
{
	return lodge_image_desc_get_offset_pixel_channel(desc, x, y, 0);
}

size_t lodge_image_desc_get_offset_pixel_g(const struct lodge_image_desc *desc, uint32_t x, uint32_t y)
{
	return lodge_image_desc_get_offset_pixel_channel(desc, x, y, 1);
}

size_t lodge_image_desc_get_offset_pixel_b(const struct lodge_image_desc *desc, uint32_t x, uint32_t y)
{
	return lodge_image_desc_get_offset_pixel_channel(desc, x, y, 2);
}

size_t lodge_image_desc_get_offset_pixel_a(const struct lodge_image_desc *desc, uint32_t x, uint32_t y)
{
	return lodge_image_desc_get_offset_pixel_channel(desc, x, y, 3);
}

struct lodge_ret lodge_image_new(struct lodge_image *image_out, const uint8_t *data, size_t len)
{
	*image_out = (struct lodge_image){ 0 };

	int width, height, channels;
	uint8_t *tmp = stbi_load_from_memory(data, len, &width, &height, &channels, 0);

	image_out->desc = (struct lodge_image_desc) {
		.width = width,
		.height = height,
		.channels = channels,
		.bytes_per_channel = 1,
	};

	if(tmp == NULL) {
		const char* stbi_error = stbi_failure_reason();
		return lodge_ret_make_error(strview_make(stbi_error, strlen(stbi_error)));
	}

	image_out->pixel_data = tmp;
	image_out->shared_pixel_data = false;
	
	return lodge_success();
}

struct lodge_ret lodge_image_new_from_blob(struct lodge_image *image_out, struct blob *blob)
{
	return lodge_image_new(image_out, (const uint8_t*)blob_data(blob), blob_size(blob));
}

void lodge_image_free(struct lodge_image *image)
{
	// NOTE(TS): may need to keep track of allocator since `lodge_image_new_solid_color` uses malloc()
	if(!image->shared_pixel_data) {
		stbi_image_free(image->pixel_data);
	}
	image->pixel_data = NULL;
	image->desc.width = 0;
	image->desc.height = 0;
	image->desc.channels = 0;
	image->desc.bytes_per_channel = 0;
}

struct lodge_ret lodge_image_new_solid_color(struct lodge_image *image_out, uint32_t width, uint32_t height, vec4 color)
{
	/* Generate the buffer containing pixel data. */
	const uint8_t components[4] = COLOR_TO_UINT8(color);

	size_t buf_size = sizeof(components) * width * height;
	uint8_t *buf = (uint8_t *) malloc(buf_size);
	if(buf == NULL) {
		return lodge_ret_make_error(strview_static("Out of memory"));
	}

	for(size_t i=0; i<buf_size; i += sizeof(components)) {
		memcpy(&buf[i], components, sizeof(components));
	}

	*image_out = (struct lodge_image) {
		.desc = (struct lodge_image_desc) {
			.width = width,
			.height = height,
			.channels = 4,
			.bytes_per_channel = 1
		},
		.pixel_data = buf
	};

	return lodge_ret_make_success();
}

typedef void (*lodge_image_downsample_func_t)(uint8_t *dst, const struct lodge_image *src, uint32_t src_x0, uint32_t src_y0, uint32_t src_x1, uint32_t src_y1);

static void lodge_image_downsample_avg(uint8_t *dst, const struct lodge_image *src, uint32_t src_x0, uint32_t src_y0, uint32_t src_x1, uint32_t src_y1)
{
	const uint8_t bytes_per_channel = src->desc.bytes_per_channel;
	for(uint8_t c = 0, c_max = src->desc.channels; c < c_max; c++) {
		float avg = 0.0f;
		for(uint32_t src_y = src_y0; src_y < src_y1; src_y++) {
			for(uint32_t src_x = src_x0; src_x < src_x1; src_x++) {
				avg += lodge_image_pixel_channel_get_01(lodge_image_get_pixel_channel(src, src_x, src_y, c), bytes_per_channel);
			}
		}
		avg /= (src_x1 - src_x0) + (src_y1 - src_y0);
		dst = lodge_image_pixel_channel_set_01(dst, src->desc.bytes_per_channel, avg);
	}
}

static void lodge_image_downsample_max(uint8_t *dst, const struct lodge_image *src, uint32_t src_x0, uint32_t src_y0, uint32_t src_x1, uint32_t src_y1)
{
	const uint8_t bytes_per_channel = src->desc.bytes_per_channel;

	for(uint8_t c = 0, c_max = src->desc.channels; c < c_max; c++) {
		float max = 0.0f;
		for(uint32_t src_y = src_y0; src_y < src_y1; src_y++) {
			for(uint32_t src_x = src_x0; src_x < src_x1; src_x++) {
				max = max(max, lodge_image_pixel_channel_get_01(lodge_image_get_pixel_channel(src, src_x, src_y, c), bytes_per_channel));
			}
		}
		dst = lodge_image_pixel_channel_set_01(dst, src->desc.bytes_per_channel, max);
	}
}

static void lodge_image_downsample_min(uint8_t *dst, const struct lodge_image *src, uint32_t src_x0, uint32_t src_y0, uint32_t src_x1, uint32_t src_y1)
{
	const uint8_t bytes_per_channel = src->desc.bytes_per_channel;

	for(uint8_t c = 0, c_max = src->desc.channels; c < c_max; c++) {
		float min = 1.0f;
		for(uint32_t src_y = src_y0; src_y < src_y1; src_y++) {
			for(uint32_t src_x = src_x0; src_x < src_x1; src_x++) {
				min = min(min, lodge_image_pixel_channel_get_01(lodge_image_get_pixel_channel(src, src_x, src_y, c), bytes_per_channel));
			}
		}
		dst = lodge_image_pixel_channel_set_01(dst, src->desc.bytes_per_channel, min);
	}
}

void lodge_image_new_downsample_inplace(struct lodge_image *dst, const struct lodge_image *src, lodge_image_downsample_func_t func)
{
	ASSERT(src);
	ASSERT(dst);

	*dst = (struct lodge_image) {
		.desc = {
			.width = src->desc.width / 2,
			.height = src->desc.height / 2,
			.channels = src->desc.channels,
			.bytes_per_channel = src->desc.bytes_per_channel,
		},
		.pixel_data = NULL,
	};
	dst->pixel_data = malloc(lodge_image_desc_get_data_size(&dst->desc));

	uint8_t *pixel_data_cursor = dst->pixel_data;
	for(uint32_t y = 0, y_max = dst->desc.height; y < y_max; y++) {
		for(uint32_t x = 0, x_max = dst->desc.width; x < x_max; x++) {
			const uint32_t src_x0 = x * 2;
			const uint32_t src_y0 = y * 2;
			const uint32_t src_x1 = x * 2 + 2;
			const uint32_t src_y1 = y * 2 + 2;

#if 0
			for(uint8_t c = 0, c_max = dst->desc.channels; c < c_max; c++) {
				float avg = 0.0f;
				avg += lodge_image_pixel_channel_get_01(src, lodge_image_get_pixel_channel(src, src_x,     src_y,     c));
				avg += lodge_image_pixel_channel_get_01(src, lodge_image_get_pixel_channel(src, src_x + 1, src_y,     c));
				avg += lodge_image_pixel_channel_get_01(src, lodge_image_get_pixel_channel(src, src_x,     src_y + 1, c));
				avg += lodge_image_pixel_channel_get_01(src, lodge_image_get_pixel_channel(src, src_x + 1, src_y + 1, c));
				avg *= 0.25f;
				lodge_image_pixel_channel_set_01(dst, pixel_data_cursor, avg);
				pixel_data_cursor += dst->desc.bytes_per_channel;
			}
#else
			func(pixel_data_cursor, src, src_x0, src_y0, src_x1, src_y1);
			pixel_data_cursor += dst->desc.bytes_per_channel;
#endif
		}
	}
}


void lodge_image_new_mipmap_inplace(struct lodge_image *dst, const struct lodge_image *src)
{
	lodge_image_new_downsample_inplace(dst, src, &lodge_image_downsample_avg);
}

void lodge_image_new_max_mipmap_inplace(struct lodge_image *dst, const struct lodge_image *src)
{
	lodge_image_new_downsample_inplace(dst, src, &lodge_image_downsample_max);
}

void lodge_image_new_min_mipmap_inplace(struct lodge_image *dst, const struct lodge_image *src)
{
	lodge_image_new_downsample_inplace(dst, src, &lodge_image_downsample_min);
}

const uint8_t* lodge_image_get_row(const struct lodge_image *image, uint32_t y)
{
	ASSERT(image);
	return image->pixel_data + lodge_image_desc_get_offset_row(&image->desc, y);
}

const uint8_t* lodge_image_get_pixel(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	ASSERT(image);
	return image->pixel_data + lodge_image_desc_get_offset_pixel(&image->desc, x, y);
}

const uint8_t* lodge_image_get_pixel_channel(const struct lodge_image *image, uint32_t x, uint32_t y, uint8_t c)
{
	ASSERT(image);
	return image->pixel_data + lodge_image_desc_get_offset_pixel_channel(&image->desc, x, y, c);
}

const uint8_t* lodge_image_get_pixel_r(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	ASSERT(image);
	return image->pixel_data + lodge_image_desc_get_offset_pixel_r(&image->desc, x, y);
}

const uint8_t* lodge_image_get_pixel_g(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	ASSERT(image);
	return image->pixel_data + lodge_image_desc_get_offset_pixel_g(&image->desc, x, y);
}

const uint8_t* lodge_image_get_pixel_b(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	ASSERT(image);
	return image->pixel_data + lodge_image_desc_get_offset_pixel_b(&image->desc, x, y);
}

float lodge_image_pixel_channel_get_01(const uint8_t *pixel_channel, uint8_t bytes_per_channel)
{
	switch(bytes_per_channel)
	{
	case 1:
		return *pixel_channel / (float)UINT8_MAX;
	case 2:
		return (*(const uint16_t*)(pixel_channel)) / (float)UINT16_MAX;
	default:
		ASSERT_FAIL("Unsupported bytes_per_channel");
		return 0.0f;
	}
}

uint8_t* lodge_image_pixel_channel_set_01(uint8_t *pixel_channel, uint8_t bytes_per_channel, float value)
{
	ASSERT(value >= 0.0f && value <= 1.0f);

	switch(bytes_per_channel)
	{
	case 1: {
		*pixel_channel = (uint8_t)(roundf(value * UINT8_MAX));
		return pixel_channel + 1;
	}
	case 2: {
		*(uint16_t*)(pixel_channel) = (uint16_t)(roundf(value * UINT16_MAX));
		return pixel_channel + 2;
	}
	default:
		ASSERT_FAIL("Unsupported bytes_per_channel");
		return pixel_channel;
	}
}

float lodge_image_get_pixel_channel_01(const struct lodge_image *image, vec2 p, uint8_t c)
{
	ASSERT(p.x >= 0.0f && p.x <= 1.0f);
	ASSERT(p.y >= 0.0f && p.y <= 1.0f);
	//
	// TODO(TS): floor instead? ({width,height} - 1) ?
	//
	const uint32_t x = (uint32_t)roundf((image->desc.width - 1) * p.x);
	const uint32_t y = (uint32_t)roundf((image->desc.height - 1) * p.y);
	const uint8_t *pixel_channel = lodge_image_get_pixel_channel(image, x, y, c);
	return lodge_image_pixel_channel_get_01(pixel_channel, image->desc.bytes_per_channel);
}

float lodge_image_get_pixel_r_01(const struct lodge_image *image, vec2 p)
{
	return lodge_image_get_pixel_channel_01(image, p, 0);
}

float lodge_image_get_pixel_g_01(const struct lodge_image *image, vec2 p)
{
	return lodge_image_get_pixel_channel_01(image, p, 1);
}

float lodge_image_get_pixel_b_01(const struct lodge_image *image, vec2 p)
{
	return lodge_image_get_pixel_channel_01(image, p, 2);
}
