#include "lodge_image.h"

#include "blob.h"
#include <stb/stb_image.h>

struct lodge_ret lodge_image_new(struct lodge_image_out *out, const uint8_t *data, size_t len)
{
	*out = (struct lodge_image_out){ 0 };

	uint8_t *tmp = stbi_load_from_memory(data, len,
		&out->width,
		&out->height,
		&out->components,
		STBI_rgb_alpha);

	if(tmp == NULL) {
		const char* stbi_error = stbi_failure_reason();
		return lodge_ret_make_error(strview_make(stbi_error, strlen(stbi_error)));
	}

	out->pixel_data = tmp;
	
	return lodge_success();
}

struct lodge_ret lodge_image_new_from_blob(struct lodge_image_out *out, struct blob *blob)
{
	return lodge_image_new(out, blob_data(blob), blob_size(blob));
}

void lodge_image_free(uint8_t *data)
{
	stbi_image_free(data);
}
