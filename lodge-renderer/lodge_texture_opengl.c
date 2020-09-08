#include "lodge_texture.h"

#include "lodge_image.h"
#include "lodge_opengl.h"

static enum lodge_texture_format lodge_texture_format_from_image(const struct lodge_image *image)
{
	if(image->desc.bytes_per_channel == 1) {
		switch(image->desc.channels)
		{
		case 1:
			return LODGE_TEXTURE_FORMAT_R8;
		case 3:
			return LODGE_TEXTURE_FORMAT_RGB8;
		case 4:
			return LODGE_TEXTURE_FORMAT_RGBA8;
		default:
			ASSERT_NOT_IMPLEMENTED();
			return LODGE_TEXTURE_FORMAT_RGBA8;
		}
	} else if(image->desc.bytes_per_channel == 2) {
		switch(image->desc.channels)
		{
		case 1:
			return LODGE_TEXTURE_FORMAT_R16;
		case 3:
			return LODGE_TEXTURE_FORMAT_RGB16;
		case 4:
			return LODGE_TEXTURE_FORMAT_RGBA16;
		default:
			ASSERT_NOT_IMPLEMENTED();
			return LODGE_TEXTURE_FORMAT_RGBA16;
		}
	} else {
		ASSERT_NOT_IMPLEMENTED();
		return LODGE_TEXTURE_FORMAT_RGBA8;
	}
}

static enum lodge_pixel_format lodge_pixel_format_from_image(const struct lodge_image *image)
{
	switch(image->desc.channels)
	{
	case 1:
		return LODGE_PIXEL_FORMAT_R;
	case 3:
		return LODGE_PIXEL_FORMAT_RGB;
	case 4:
		return LODGE_PIXEL_FORMAT_RGBA;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return LODGE_PIXEL_FORMAT_RGBA;
	}
}

static enum lodge_pixel_format lodge_pixel_type_from_image(const struct lodge_image *image)
{
	switch(image->desc.bytes_per_channel)
	{
	case 1:
		return LODGE_PIXEL_TYPE_UINT8;
	case 2:
		return LODGE_PIXEL_TYPE_UINT16;
	case 4:
		return LODGE_PIXEL_TYPE_FLOAT;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return LODGE_PIXEL_TYPE_UINT8;
	}
}

static uint32_t lodge_texture_calc_num_levels(uint32_t width, uint32_t height, uint32_t depth)
{
	return 1 + (uint32_t)floor(log2(max(max(width, height), depth)));
}

static GLenum lodge_texture_format_to_gl(enum lodge_texture_format texture_format)
{
	switch(texture_format)
	{
	case LODGE_TEXTURE_FORMAT_R16:
		return GL_R16;
	case LODGE_TEXTURE_FORMAT_R32F:
		return GL_R32F;
	case LODGE_TEXTURE_FORMAT_RGB8:
		return GL_RGB8;
	case LODGE_TEXTURE_FORMAT_RGB16:
		return GL_RGB16;
	case LODGE_TEXTURE_FORMAT_RGBA8:
		return GL_RGBA8;
	case LODGE_TEXTURE_FORMAT_RGBA16:
		return GL_RGBA16;
	case LODGE_TEXTURE_FORMAT_RGBA16F:
		return GL_RGBA16F;
	case LODGE_TEXTURE_FORMAT_RGBA32F:
		return GL_RGBA32F;
	case LODGE_TEXTURE_FORMAT_DEPTH16:
		return GL_DEPTH_COMPONENT16;
	case LODGE_TEXTURE_FORMAT_DEPTH32:
		return GL_DEPTH_COMPONENT32;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_RGBA8;
	}
}

static GLenum lodge_pixel_format_to_gl(enum lodge_pixel_format pixel_format)
{
	switch(pixel_format)
	{
	case LODGE_PIXEL_FORMAT_R:
		return GL_RED;
	case LODGE_PIXEL_FORMAT_RGB:
		return GL_RGB;
	case LODGE_PIXEL_FORMAT_RGBA:
		return GL_RGBA;
	case LODGE_PIXEL_FORMAT_DEPTH:
		return GL_DEPTH_COMPONENT;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_RGBA;
	}
}

static GLenum lodge_pixel_type_to_gl(enum lodge_pixel_type pixel_type)
{
	switch(pixel_type)
	{
	case LODGE_PIXEL_TYPE_UINT8:
		return GL_UNSIGNED_BYTE;
	case LODGE_PIXEL_TYPE_UINT16:
		return GL_UNSIGNED_SHORT;
	case LODGE_PIXEL_TYPE_INT8:
		return GL_BYTE;
	case LODGE_PIXEL_TYPE_INT16:
		return GL_SHORT;
	case LODGE_PIXEL_TYPE_FLOAT:
		return GL_FLOAT;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_UNSIGNED_BYTE;
	}
}

lodge_texture_t lodge_texture_2d_make(struct lodge_texture_2d_desc desc)
{
	GLuint texture = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	GL_OK_OR_GOTO(fail);

	const uint32_t mipmaps_count = desc.mipmaps_count != 0 ? desc.mipmaps_count : lodge_texture_calc_num_levels(desc.width, desc.height, 1);

	glTextureStorage2D(texture, mipmaps_count, lodge_texture_format_to_gl(desc.texture_format), desc.width, desc.height);
	GL_OK_OR_GOTO(fail);

	if(desc.data) {
		glTextureSubImage2D(texture, 0, 0, 0, desc.width, desc.height, lodge_pixel_format_to_gl(desc.pixel_format), lodge_pixel_type_to_gl(desc.pixel_type), desc.data);
		GL_OK_OR_GOTO(fail);
	}

	if(mipmaps_count > 1) {
		glGenerateTextureMipmap(texture);
		GL_OK_OR_GOTO(fail);
	}

	return lodge_texture_from_gl(texture);

fail:
	ASSERT_FAIL("Failed to make texture");
	return NULL;
}

lodge_texture_t lodge_texture_2d_make_from_image(const struct lodge_image *image)
{
	return lodge_texture_2d_make(lodge_texture_2d_desc_make_from_image(image));
}

lodge_texture_t lodge_texture_2d_make_rgba(uint32_t width, uint32_t height)
{
	return lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		
		.mipmaps_count = 1,
	
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA8,
		.pixel_format = LODGE_PIXEL_FORMAT_RGBA,
		.pixel_type = LODGE_PIXEL_TYPE_UINT8,
	
		.data = NULL
	});
}

lodge_texture_t lodge_texture_2d_make_depth(uint32_t width, uint32_t height)
{
	return lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		
		.mipmaps_count = 1,
	
		.texture_format = LODGE_TEXTURE_FORMAT_DEPTH32,
		.pixel_format = LODGE_PIXEL_FORMAT_DEPTH,
		.pixel_type = LODGE_PIXEL_TYPE_FLOAT,
	
		.data = NULL
	});
}

struct lodge_texture_2d_desc lodge_texture_2d_desc_make_from_image(const struct lodge_image *image)
{
	ASSERT(image);
	return (struct lodge_texture_2d_desc) {
		.width = image->desc.height,
		.height = image->desc.width,
		
		.mipmaps_count = 0,

		.texture_format = lodge_texture_format_from_image(image),
		.pixel_format = lodge_pixel_format_from_image(image),
		.pixel_type = lodge_pixel_type_from_image(image),

		.data = image->pixel_data,
	};
}

static bool lodge_texture_is_power_of_2(uint32_t width, uint32_t height)
{
	return ((width & (width - 1)) == 0) && ((height & (height - 1)) == 0);
}

static bool lodge_texture_cubemap_load_side(GLuint texture, GLenum side, const struct lodge_texture_2d_desc desc)
{
	ASSERT(lodge_texture_is_power_of_2(desc.width, desc.height));

	// copy image data into 'target' side of cube map
	//glTexImage2D(side, 0, format.internal_format, image->desc.width, image->desc.height, 0, format.pixel_format, format.channel_type, image->pixel_data);
	glTextureSubImage3D(texture, 0, 0, 0, side, desc.width, desc.height, 1, lodge_pixel_format_to_gl(desc.pixel_format), lodge_pixel_type_to_gl(desc.pixel_type), desc.data);
	GL_OK_OR_RETURN(false);

	return true;
}

struct lodge_texture_2d_array_desc
{
	uint32_t					width;
	uint32_t					height;
	uint32_t					depth;

	uint32_t					mipmaps_count; // if 0 = calc default

	enum lodge_texture_format	texture_format;
	enum lodge_pixel_format		pixel_format;
	enum lodge_pixel_data_type	pixel_type;

	//const void				*data;
};

static lodge_texture_t lodge_texture_2d_array_make(struct lodge_texture_2d_array_desc desc)
{
	GLuint texture = 0;
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture);
	GL_OK_OR_GOTO(fail);

	const uint32_t levels_count = desc.mipmaps_count != 0 ? desc.mipmaps_count : lodge_texture_calc_num_levels(desc.width, desc.height, 1);

	glTextureStorage3D(texture, desc.mipmaps_count, lodge_texture_format_to_gl(desc.texture_format), desc.width, desc.height, desc.depth);
	GL_OK_OR_GOTO(fail);

#if 0
	if(desc.data) {
		glTextureSubImage3D(texture, 0, 0, 0, desc.width, desc.height, lodge_pixel_format_to_gl(desc.pixel_format), lodge_pixel_type_to_gl(desc.pixel_type), desc.data);
		GL_OK_OR_GOTO(fail);
	}
#endif

	return lodge_texture_from_gl(texture);

fail:
	ASSERT_FAIL("Failed to make texture");
	return NULL;
}

lodge_texture_t lodge_texture_2d_array_make_depth(uint32_t width, uint32_t height, uint32_t depth)
{
	return lodge_texture_2d_array_make((struct lodge_texture_2d_array_desc) {
		.width = width,
		.height = height,
		.depth = depth,

		.mipmaps_count = 1,
	
		.texture_format = LODGE_TEXTURE_FORMAT_DEPTH32,
		.pixel_format = LODGE_PIXEL_FORMAT_DEPTH,
		.pixel_type = LODGE_PIXEL_TYPE_FLOAT,

#if 0
		.data = NULL
#endif
	});
}

enum
{
	LODGE_CUBEMAP_X_POS,
	LODGE_CUBEMAP_X_NEG,
	LODGE_CUBEMAP_Y_POS,
	LODGE_CUBEMAP_Y_NEG,
	LODGE_CUBEMAP_Z_POS,
	LODGE_CUBEMAP_Z_NEG,
};

lodge_texture_t lodge_texture_cubemap_make(struct lodge_texture_cubemap_desc desc)
{
	GLuint texture;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);
	GL_OK_OR_GOTO(fail);

	glTextureStorage2D(texture, 1, GL_RGBA8, desc.top->desc.width, desc.top->desc.height);
	GL_OK_OR_GOTO(fail);

	if(desc.front && !lodge_texture_cubemap_load_side(texture, LODGE_CUBEMAP_Y_POS, lodge_texture_2d_desc_make_from_image(desc.front))) {
		goto fail;
	}
	if(desc.back && !lodge_texture_cubemap_load_side(texture, LODGE_CUBEMAP_Y_NEG, lodge_texture_2d_desc_make_from_image(desc.back))) {
		goto fail;
	}
	if(desc.top && !lodge_texture_cubemap_load_side(texture, LODGE_CUBEMAP_Z_POS, lodge_texture_2d_desc_make_from_image(desc.top))) {
		goto fail;
	}
	if(desc.bottom && !lodge_texture_cubemap_load_side(texture, LODGE_CUBEMAP_Z_NEG, lodge_texture_2d_desc_make_from_image(desc.bottom))) {
		goto fail;
	}
	if(desc.left && !lodge_texture_cubemap_load_side(texture, LODGE_CUBEMAP_X_NEG, lodge_texture_2d_desc_make_from_image(desc.left))) {
		goto fail;
	}
	if(desc.right && !lodge_texture_cubemap_load_side(texture, LODGE_CUBEMAP_X_POS, lodge_texture_2d_desc_make_from_image(desc.right))) {
		goto fail;
	}

	return lodge_texture_from_gl(texture);

fail:
	ASSERT_FAIL("Failed to make cubemap texture");
	return NULL;
}

void lodge_texture_reset(lodge_texture_t texture)
{
	if(!texture) {
		return;
	}

	glDeleteTextures(1, &(GLuint){ lodge_texture_to_gl(texture) });
	GL_OK_OR_ASSERT("Failed to reset texture");
}
