#ifndef _LODGE_IMAGE_RAW_H
#define _LODGE_IMAGE_RAW_H

#include "lodge_image.h"

#include <stdbool.h>

bool	lodge_image_raw_new(struct lodge_image *image, const struct lodge_image_desc *desc, const uint8_t *data, size_t size);

size_t	lodge_image_raw_expected_size(const struct lodge_image_desc *desc);

#endif