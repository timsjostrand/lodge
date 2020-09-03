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
	
	return lodge_success();
}

struct lodge_ret lodge_image_new_from_blob(struct lodge_image *image_out, struct blob *blob)
{
	return lodge_image_new(image_out, blob_data(blob), blob_size(blob));
}

void lodge_image_free(struct lodge_image *image)
{
	// NOTE(TS): may need to keep track of allocator since `lodge_image_new_solid_color` uses malloc()
	stbi_image_free(image->pixel_data);
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

const uint8_t* lodge_image_get_row(const struct lodge_image *image, uint32_t y)
{
	ASSERT(image && image->pixel_data);
	ASSERT(y < image->desc.height);
	const uint32_t stride = image->desc.width * image->desc.channels * image->desc.bytes_per_channel;
	return image->pixel_data + y * stride;
}

const uint8_t* lodge_image_get_pixel(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	ASSERT(x < image->desc.width);
	return lodge_image_get_row(image, y) + x * image->desc.channels * image->desc.bytes_per_channel;
}

const uint8_t* lodge_image_get_pixel_channel(const struct lodge_image *image, uint32_t x, uint32_t y, uint8_t c)
{
	ASSERT(c < image->desc.channels);
	return lodge_image_get_pixel(image, x, y) + c * image->desc.bytes_per_channel;
}

const uint8_t* lodge_image_get_pixel_r(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	return lodge_image_get_pixel_channel(image, x, y, 0);
}

const uint8_t* lodge_image_get_pixel_g(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	return lodge_image_get_pixel_channel(image, x, y, 1);
}

const uint8_t* lodge_image_get_pixel_b(const struct lodge_image *image, uint32_t x, uint32_t y)
{
	return lodge_image_get_pixel_channel(image, x, y, 2);
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

	switch(image->desc.bytes_per_channel)
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
