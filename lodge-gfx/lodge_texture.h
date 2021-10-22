#ifndef _LODGE_TEXTURE_H
#define _LODGE_TEXTURE_H

#include "lodge_platform.h"
#include "lodge_gfx_types.h"

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

enum lodge_texture_format
{
	LODGE_TEXTURE_FORMAT_R8,
	LODGE_TEXTURE_FORMAT_R16,
	LODGE_TEXTURE_FORMAT_RGB8,
	LODGE_TEXTURE_FORMAT_RGB16,
	LODGE_TEXTURE_FORMAT_RGB16F,
	LODGE_TEXTURE_FORMAT_RGBA8,
	LODGE_TEXTURE_FORMAT_RGBA16,
	LODGE_TEXTURE_FORMAT_RGBA16F,
	LODGE_TEXTURE_FORMAT_RGBA32F,
	LODGE_TEXTURE_FORMAT_R32F,
	LODGE_TEXTURE_FORMAT_DEPTH16,
	LODGE_TEXTURE_FORMAT_DEPTH32,
};

enum lodge_pixel_format
{
	LODGE_PIXEL_FORMAT_R,
	LODGE_PIXEL_FORMAT_RGB,
	LODGE_PIXEL_FORMAT_RGBA,
	LODGE_PIXEL_FORMAT_DEPTH,
	//LODGE_PIXEL_FORMAT_STENCIL,
};

enum lodge_pixel_data_type
{
	LODGE_PIXEL_TYPE_UINT8,
	LODGE_PIXEL_TYPE_UINT16,
	LODGE_PIXEL_TYPE_INT8,
	LODGE_PIXEL_TYPE_INT16,
	LODGE_PIXEL_TYPE_FLOAT,
};

struct lodge_texture_2d_desc
{
	uint32_t					width;
	uint32_t					height;
	uint32_t					mipmaps_count; // if 0 = calc default
	enum lodge_texture_format	texture_format;
};

struct lodge_texture_2d_array_desc
{
	uint32_t					width;
	uint32_t					height;
	uint32_t					depth;

	uint32_t					mipmaps_count; // if 0 = calc default

	enum lodge_texture_format	texture_format;
};

struct lodge_texture_data_desc
{
	enum lodge_pixel_format		pixel_format;
	enum lodge_pixel_data_type	pixel_type;

	const void					*data;
};

struct lodge_texture_3d_desc
{
	uint32_t					width;
	uint32_t					height;
	uint32_t					depth;
	enum lodge_texture_format	texture_format;
};

struct lodge_image;

struct lodge_texture_cubemap_desc
{
	const struct lodge_image	*front;
	const struct lodge_image	*back;
	const struct lodge_image	*top;
	const struct lodge_image	*bottom;
	const struct lodge_image	*left;
	const struct lodge_image	*right;
};

//
// TODO(TS): move/remove lodge_image related API from lodge_texture: work only on texture_XX_desc and texture_data_desc
//
struct lodge_texture_2d_desc	lodge_texture_2d_desc_make_from_image(const struct lodge_image *image);
lodge_texture_t					lodge_texture_2d_make_from_image(const struct lodge_image *image);

lodge_texture_t					lodge_texture_2d_make(struct lodge_texture_2d_desc desc);
lodge_texture_t					lodge_texture_2d_make_from_data(struct lodge_texture_2d_desc *desc, struct lodge_texture_data_desc *data_desc);
lodge_texture_t					lodge_texture_2d_make_rgba(uint32_t width, uint32_t height);
lodge_texture_t					lodge_texture_2d_make_depth(uint32_t width, uint32_t height);

lodge_texture_t					lodge_texture_2d_array_make(struct lodge_texture_2d_array_desc *desc);
lodge_texture_t					lodge_texture_2d_array_make_depth(uint32_t width, uint32_t height, uint32_t depth);

bool							lodge_texture_2d_set_data(lodge_texture_t texture, struct lodge_texture_2d_desc *texture_desc, struct lodge_texture_data_desc *data_desc, bool generate_mipmaps);

lodge_texture_t					lodge_texture_3d_make(struct lodge_texture_3d_desc desc);

lodge_texture_t					lodge_texture_cubemap_make(struct lodge_texture_cubemap_desc desc);

void							lodge_texture_reset(lodge_texture_t texture);

lodge_texture_t					lodge_texture_view_make(lodge_texture_t texture, enum lodge_texture_target target, enum lodge_texture_format format, uint32_t levels_min, uint32_t levels_count, uint32_t layers_min, uint32_t layers_count);

void							lodge_gfx_bind_texture_2d_output(int slot, const lodge_texture_t texture, uint32_t level, enum lodge_texture_format texture_format);
void							lodge_gfx_bind_texture_3d_output(int slot, const lodge_texture_t texture, enum lodge_texture_format texture_format);

#endif