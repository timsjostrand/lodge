/**
 * Minimal routines for loading an OpenGL texture.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <inttypes.h>
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "texture.h"
#include "graphics.h"
#include "color.h"

/**
 * Parses pixel data from a compressed image.
 *
 * @param out		Where to store pixel data.
 * @param width		Where to store image width.
 * @param height	Where to store image height.
 * @param data		The compressed image data to parse.
 * @param len		Length of the compressed data.
 */
int image_load(uint8_t **out, int *width, int *height, const uint8_t *data, size_t len)
{
	int components;
	uint8_t *tmp = stbi_load_from_memory(data, len, width, height, &components, STBI_rgb_alpha);
	if(tmp == NULL) {
		graphics_error("image_load(): %s\n", stbi_failure_reason());
		return GRAPHICS_IMAGE_LOAD_ERROR;
	}
	*out = tmp;
	return GRAPHICS_OK;
}

/**
 * When an image has been loaded via image_load{_file}(), use image_free() to release
 * it.
 *
 * @param data	The data to release.
 */
void image_free(uint8_t *data)
{
	stbi_image_free(data);
}

/**
 * Loads raw RGBA data into an OpenGL texture.
 *
 * @param tex		Where to store the texture id.
 * @param data		The image data to upload.
 * @param width		The width of the image.
 * @param height	The height of the image.
 */
int texture_load_pixels(GLuint *tex, const uint8_t *data,
		const int width, const int height)
{
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	/* Upload texture. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
	/* Wrapping. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	/* Filter (mipmap). */
//	glGenerateMipmap(GL_TEXTURE_2D);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return GRAPHICS_OK;
}

/**
 * Generates a new texture filled with only one color.
 *
 * @param tex	Where to store the texture.
 * @param w		The width of the texture.
 * @param h		The height of the texture.
 * @param color	The color of the texture.
 */
int texture_solid_color(GLuint *tex, int w, int h, const GLfloat color[4])
{
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	/* Generate the buffer containing pixel data. */
	GLfloat *buf = (GLfloat *) malloc(sizeof(color) * w * h);

	/* OOM */
	if(buf == NULL) {
		return TEXTURE_ERROR;
	}

	for(GLfloat *cur = buf, *end = buf + sizeof(color) * w * h;
			cur < end;
			cur += sizeof(color)) {
		cur[0] = color[0];
		cur[1] = color[1];
		cur[2] = color[2];
		cur[3] = color[3];
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
void texture_white(GLuint *tex)
{
	texture_solid_color(tex, 1, 1, COLOR_WHITE);
}

void texture_free(const GLuint tex)
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
int texture_load(GLuint *tex, int *width, int *height, const uint8_t *data,
		size_t len)
{
	uint8_t *tmp;
	int tmp_width;
	int tmp_height;
	int ret;
	ret = image_load(&tmp, &tmp_width, &tmp_height, data, len);
	if(ret != GRAPHICS_OK) {
		return ret;
	}
	ret = texture_load_pixels(tex, tmp, tmp_width, tmp_height);
	image_free(tmp);
	if(ret != GRAPHICS_OK) {
		texture_free(*tex);
		return ret;
	}
	if(width != NULL) {
		*(width) = tmp_width;
	}
	if(height != NULL) {
		*(height) = tmp_height;
	}
	return GRAPHICS_OK;
}
