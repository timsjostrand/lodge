/**
 * Minimal routines for loading an OpenGL texture.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <inttypes.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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
 * @param data		Store the pixel data from the image here.
 * @param width		Store the width of the image here.
 * @param height	Store the height of the image here.
 * @param path		The path to load image data from.
 */
int image_load_file(uint8_t **data, int *width, int *height, const char *path)
{
	int components;
	uint8_t *tmp = stbi_load(path, width, height, &components, STBI_rgb_alpha);
	if(tmp == NULL) {
		graphics_error("image_load_file(): %s\n", stbi_failure_reason());
		return GRAPHICS_IMAGE_LOAD_ERROR;
	}
	*data = tmp;
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	/* Filter (mipmap). */
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	return GRAPHICS_OK;
}

/**
 * Creates an empty texture, a white square. If a sprite does not have a
 * texture, the fragment shader can still multiply this texture with a sprite
 * color to produce the correct fragment color.
 */
void texture_white(GLuint *tex)
{
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	/* Upload texture. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
			GL_FLOAT, COLOR_WHITE);
	/* Wrapping. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	/* Filter. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void texture_free(const GLuint tex)
{
	glDeleteTextures(1, &tex);
}

/**
 * Parses the image data from a PNG, JPEG, BMP or TGA image and stores in an
 * OpenGL texture.
 *
 * @param tex	Store the OpenGL texture id here.
 * @param data	The image data to load.
 * @param len	The length of the image data.
 */
int texture_load(GLuint *tex, const uint8_t *data, size_t len)
{
	uint8_t *tmp;
	int width;
	int height;
	int ret;
	ret = image_load(&tmp, &width, &height, data, len);
	if(ret != GRAPHICS_OK) {
		return ret;
	}
	ret = texture_load_pixels(tex, tmp, width, height);
	image_free(tmp);
	if(ret != GRAPHICS_OK) {
		texture_free(*tex);
		return ret;
	}
	return GRAPHICS_OK;
}

/**
 * @param tex	Store the OpenGL texture id here.
 * @param path	Where to load the image from.
 */
int texture_load_file(GLuint *tex, const char *path)
{
	uint8_t *data;
	int width;
	int height;
	int ret;
	ret = image_load_file(&data, &width, &height, path);
	if(ret != GRAPHICS_OK) {
		return ret;
	}
	ret = texture_load_pixels(tex, data, width, height);
	image_free(data);
	if(ret != GRAPHICS_OK) {
		texture_free(*tex);
		return ret;
	}
	return GRAPHICS_OK;
}
