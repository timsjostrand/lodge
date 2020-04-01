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

	uint8_t *tmp = stbi_load_from_memory(data, len,
		&image_out->width,
		&image_out->height,
		&image_out->components,
		STBI_rgb_alpha);

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
	image->width = 0;
	image->height = 0;
	image->components = 0;
}

struct lodge_ret lodge_image_new_solid_color(struct lodge_image *image_out, int width, int height, vec4 color)
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
		.width = width,
		.height = height,
		.components = 4,
		.pixel_data = buf
	};

	return lodge_ret_make_success();
}
