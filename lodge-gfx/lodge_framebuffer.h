#ifndef _LODGE_FRAMEBUFFER_H
#define _LODGE_FRAMEBUFFER_H

#include "math4.h"
#include "lodge_rect.h"

#include <stdint.h>

#define LODGE_FRAMEBUFFER_DEPTH_DEFAULT		1.0f
#define LODGE_FRAMEBUFFER_STENCIL_DEFAULT	0

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

struct lodge_framebuffer_desc
{
	uint32_t			colors_count;
	
	lodge_texture_t		colors[16];
	uint32_t			color_levels[16];
	
	lodge_texture_t		depth;
	uint32_t			depth_level;
	
	lodge_texture_t		stencil;
	uint32_t			stencil_level;
};

lodge_framebuffer_t		lodge_framebuffer_make(struct lodge_framebuffer_desc *desc);
void					lodge_framebuffer_reset(lodge_framebuffer_t framebuffer);

lodge_framebuffer_t		lodge_framebuffer_default();

void					lodge_framebuffer_bind(lodge_framebuffer_t framebuffer);
void					lodge_framebuffer_unbind();

void					lodge_framebuffer_clear_color(lodge_framebuffer_t framebuffer, uint32_t index, vec4 clear_value);
void					lodge_framebuffer_clear_depth(lodge_framebuffer_t framebuffer, float depth_value);
void					lodge_framebuffer_clear_stencil(lodge_framebuffer_t framebuffer, int32_t stencil_value);
#if 0
void					lodge_framebuffer_clear_depth_stencil(lodge_framebuffer_t framebuffer, float depth_value, int32_t stencil_value);
#endif

void					lodge_framebuffer_set_depth_layer(lodge_framebuffer_t framebuffer, lodge_texture_t texture, uint32_t layer);

void					lodge_framebuffer_copy(lodge_framebuffer_t dst, lodge_framebuffer_t src, struct lodge_recti dst_rect, struct lodge_recti src_rect);

vec4					lodge_framebuffer_read_pixel_rgba(uint32_t color_index, uint32_t x, uint32_t y);

#endif // _FRAMEBUFFER_H
