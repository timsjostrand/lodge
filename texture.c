/**
 * Minimal routines for loading an OpenGL texture.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <GL/glew.h>
#include <stb/stb_image.h>

#include "texture.h"
#include "color.h"
#include "lodge_image.h"

/**
 * Loads raw RGBA data into an OpenGL texture.
 *
 * @param tex		Where to store the texture id.
 * @param data		The image data to upload.
 * @param width		The width of the image.
 * @param height	The height of the image.
 */
int texture_load_pixels(tex_t *tex, const uint8_t *data,
		const int width, const int height)
{
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	/* Upload texture. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
#if 0
	/* Linear mipmapped */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
#else
	/* Nearest */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
	return TEXTURE_OK;
}

/**
 * Generates a new texture filled with only one color.
 *
 * @param tex	Where to store the texture.
 * @param w		The width of the texture.
 * @param h		The height of the texture.
 * @param color	The color of the texture.
 */
int texture_solid_color(tex_t *tex, int w, int h, const vec4 color)
{
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	/* Generate the buffer containing pixel data. */
	size_t buf_size = sizeof(vec4) * w * h;
	GLfloat *buf = (GLfloat *) malloc(buf_size);

	/* OOM */
	if(buf == NULL) {
		return TEXTURE_ERROR;
	}

	for(size_t i=0; i<buf_size; i += sizeof(vec4)) {
		memcpy(&buf[i], color.v, sizeof(vec4));
	}

	/* Upload texture. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
			GL_FLOAT, buf);

	/* Wrapping. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* Filter. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//free(buf);
	return TEXTURE_OK;
}

/**
 * Creates an empty texture, a white square. If a sprite does not have a
 * texture, the fragment shader can still multiply this texture with a sprite
 * color to produce the correct fragment color.
 */
void texture_white(tex_t *tex)
{
	texture_solid_color(tex, 1, 1, COLOR_WHITE);
}

void texture_free(const tex_t tex)
{
	glDeleteTextures(1, &tex);
}

/**
 * Parses the image data from a PNG, JPEG, BMP or TGA image and stores in an
 * OpenGL texture.
 *
 * @param tex		Store the OpenGL texture id here.
 * @param width		Store the width of the texture here (or NULL).
 * @param height	Store the height of the texture here (or NULL).
 * @param data		The image data to load.
 * @param len		The length of the image data.
 */
int texture_load(tex_t *tex, int *width, int *height, const uint8_t *data, size_t data_size)
{
	struct lodge_image_out image;
	struct lodge_ret image_ret = lodge_image_new(&image, data, data_size);
	if(!image_ret.success) {
		return TEXTURE_ERROR;
	}
	int ret = texture_load_pixels(tex, image.pixel_data, image.width, image.height);
	lodge_image_free(image.pixel_data);
	if(ret != TEXTURE_OK) {
		texture_free(*tex);
		return ret;
	}
	if(width != NULL) {
		*(width) = image.width;
	}
	if(height != NULL) {
		*(height) = image.height;
	}
	return TEXTURE_OK;
}
