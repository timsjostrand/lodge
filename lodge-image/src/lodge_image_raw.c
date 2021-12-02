#include "lodge_image_raw.h"

#include "log.h"

bool lodge_image_raw_new(struct lodge_image *image, const struct lodge_image_desc *desc, const uint8_t *data, size_t size)
{
	const size_t expected_size = lodge_image_raw_expected_size(desc);
	if(size != expected_size) {
		errorf("RAW Image", "Expected %zu bytes, got: %zu bytes\n", expected_size, size);
		return false;
	}

	image->desc = *desc;

	// FIXME(TS): constness and memory ownership, allocated vs inplace
	image->pixel_data = (uint8_t*)data;
	image->shared_pixel_data = true;

	return true;
}

size_t lodge_image_raw_expected_size(const struct lodge_image_desc *desc)
{
	return desc->width * desc->height * desc->channels * desc->bytes_per_channel;
}
