#include "lodge_texture.h"

#include "lodge_image.h"
#include "lodge_opengl.h"

static lodge_texture_t lodge_texture_make_details(uint32_t width, uint32_t height, GLint internal_format, GLint format, GLenum type)
{
	GLuint name;

	glGenTextures(1, &name);
	GL_OK_OR_ASSERT("glGenTextures failed");

	glBindTexture(GL_TEXTURE_2D, name);
	GL_OK_OR_ASSERT("glBindTexture failed");

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, 0);
	GL_OK_OR_ASSERT("glTextImage2D failed");

	// FIXME(TS): optional
	glGenerateMipmap(GL_TEXTURE_2D);
	GL_OK_OR_RETURN(0);

	return lodge_texture_from_gl(name);
}

struct gl_pixel_format
{
	GLint	internal_format;
	GLenum	pixel_format;
	GLenum	channel_type;
};

static GLenum bytes_per_channel_to_gl(uint8_t channel_size)
{
	switch(channel_size)
	{
	case 1:
		return GL_UNSIGNED_BYTE;
	case 2:
		return GL_UNSIGNED_SHORT;
	default:
		ASSERT_FAIL("Unsupported bytes per channel");
		return GL_INVALID_ENUM;
	}
}

static struct gl_pixel_format lodge_image_to_gl_pixel_format(const struct lodge_image *image)
{
	switch(image->desc.channels)
	{
	case 1:
		return (struct gl_pixel_format) {
			.internal_format = image->desc.bytes_per_channel == 2 ? GL_R16 : GL_R8,
			.pixel_format = GL_RED,
			.channel_type = bytes_per_channel_to_gl(image->desc.bytes_per_channel),
		};
	case 3:
		return (struct gl_pixel_format) {
			.internal_format = GL_RGB,
			.pixel_format = GL_RGB,
			.channel_type = bytes_per_channel_to_gl(image->desc.bytes_per_channel),
		};
	case 4:
		return (struct gl_pixel_format) {
			.internal_format = GL_RGBA,
			.pixel_format = GL_RGBA,
			.channel_type = bytes_per_channel_to_gl(image->desc.bytes_per_channel),
		};
	default:
		ASSERT_FAIL("Unsupported pixel format");
		return (struct gl_pixel_format) { 0 };
	}
}

static lodge_texture_t lodge_texture_make_from_pixels(const uint8_t *data, struct gl_pixel_format format, uint32_t width, uint32_t height)
{
	GLuint gl_tex;
	glGenTextures(1, &gl_tex);
	GL_OK_OR_RETURN(0);

	glBindTexture(GL_TEXTURE_2D, gl_tex);
	GL_OK_OR_RETURN(0);

	/* Upload texture. */
	glTexImage2D(GL_TEXTURE_2D, 0, format.internal_format, width, height, 0, format.pixel_format, format.channel_type, data);
	GL_OK_OR_RETURN(0);

	// FIXME(TS): optional
	glGenerateMipmap(GL_TEXTURE_2D);
	GL_OK_OR_RETURN(0);

	return lodge_texture_from_gl(gl_tex);
}

lodge_texture_t lodge_texture_make()
{
	GLuint name;
	glGenTextures(1, &name);
	GL_OK_OR_ASSERT("glGenTextures failed");
	return lodge_texture_from_gl(name);
}

lodge_texture_t lodge_texture_make_rgba(uint32_t width, uint32_t height)
{
	return lodge_texture_make_details(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
}

lodge_texture_t lodge_texture_make_depth(uint32_t width, uint32_t height)
{
	return lodge_texture_make_details(width, height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
}

lodge_texture_t lodge_texture_make_from_image(const struct lodge_image *image)
{
	struct gl_pixel_format format = lodge_image_to_gl_pixel_format(image);
	return lodge_texture_make_from_pixels(image->pixel_data, format, image->desc.width, image->desc.height);
}

static bool lodge_texture_load_cubemap_side(const struct lodge_image *image, GLenum side)
{
	if(!image) {
		return false;
	}

	// non-power-of-2 dimensions check
	if((image->desc.width & (image->desc.width - 1)) != 0 || (image->desc.height & (image->desc.height - 1)) != 0) {
		ASSERT_FAIL("Cubemap texture is not power-of-2 dimensions");
	}

	// copy image data into 'target' side of cube map
	struct gl_pixel_format format = lodge_image_to_gl_pixel_format(image);
	glTexImage2D(side, 0, format.internal_format, image->desc.width, image->desc.height, 0, format.pixel_format, format.channel_type, image->pixel_data);
	GL_OK_OR_RETURN(false);

	return true;
}

lodge_texture_t lodge_texture_make_cubemap(struct lodge_texture_cubemap_desc desc)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	GL_OK_OR_GOTO(fail);

	if(desc.front && !lodge_texture_load_cubemap_side(desc.front, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)) {
		goto fail;
	}
	if(desc.back && !lodge_texture_load_cubemap_side(desc.back, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)) {
		goto fail;
	}
	if(desc.top && !lodge_texture_load_cubemap_side(desc.top, GL_TEXTURE_CUBE_MAP_POSITIVE_Z)) {
		goto fail;
	}
	if(desc.bottom && !lodge_texture_load_cubemap_side(desc.bottom, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
		goto fail;
	}
	if(desc.left && !lodge_texture_load_cubemap_side(desc.left, GL_TEXTURE_CUBE_MAP_NEGATIVE_X)) {
		goto fail;
	}
	if(desc.right && !lodge_texture_load_cubemap_side(desc.right, GL_TEXTURE_CUBE_MAP_POSITIVE_X)) {
		goto fail;
	}

	return lodge_texture_from_gl(texture);

fail:
	ASSERT_FAIL("Failed to make cubemap texture");
	return NULL;
}

void lodge_texture_reset(lodge_texture_t *texture)
{
	GLuint name = lodge_texture_to_gl(*texture);

	ASSERT(name > 0);
	glDeleteTextures(1, &name);
	GL_OK_OR_ASSERT("Failed to delete texture");
	texture = NULL;
}

int lodge_texture_is_valid(lodge_texture_t texture)
{
	return lodge_texture_to_gl(texture) != 0;
}