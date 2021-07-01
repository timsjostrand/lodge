#include "lodge_geometry_buffer.h"

#include "lodge_framebuffer.h"
#include "lodge_texture.h"

struct lodge_geometry_buffer lodge_geometry_buffer_make(uint32_t width, uint32_t height)
{
	struct lodge_geometry_buffer gbuffer = { 0 };

	gbuffer.albedo = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = 1,
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA16F,
	});
	gbuffer.normals = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
		.width = width,
		.height = height,
		.mipmaps_count = 1,
		.texture_format = LODGE_TEXTURE_FORMAT_RGBA16F,
	});
	gbuffer.depth = lodge_texture_2d_make_depth(width, height);

	gbuffer.framebuffer = lodge_framebuffer_make((struct lodge_framebuffer_desc) {
		.colors_count = 2,
		.colors = {
			gbuffer.albedo,
			gbuffer.normals,
		},
		.depth = gbuffer.depth,
		.stencil = NULL
	});

	return gbuffer;
}

void lodge_geometry_buffer_reset(struct lodge_geometry_buffer *gbuffer)
{
	lodge_texture_reset(gbuffer->depth);
	lodge_texture_reset(gbuffer->albedo);
	lodge_texture_reset(gbuffer->normals);
	lodge_framebuffer_reset(gbuffer->framebuffer);
}

void lodge_geometry_buffer_remake(struct lodge_geometry_buffer *gbuffer, uint32_t width, uint32_t height)
{
	lodge_geometry_buffer_reset(gbuffer);
	*gbuffer = lodge_geometry_buffer_make(width, height);
}