#ifndef _LODGE_FRAMEBUFFER_H
#define _LODGE_FRAMEBUFFER_H

#include "math4.h"

#include <stdint.h>

#define LODGE_FRAMEBUFFER_DEPTH_DEFAULT		1.0f
#define LODGE_FRAMEBUFFER_STENCIL_DEFAULT	0

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

enum framebuffer_target
{
	FRAMEBUFFER_TARGET_COLOR = 0,
	FRAMEBUFFER_TARGET_DEPTH,
	FRAMEBUFFER_TARGET_STENCIL
};

lodge_framebuffer_t	lodge_framebuffer_make();
void				lodge_framebuffer_reset(lodge_framebuffer_t framebuffer);

lodge_framebuffer_t	lodge_framebuffer_default();

void				lodge_framebuffer_attach_texture(lodge_framebuffer_t framebuffer, const lodge_texture_t texture, enum framebuffer_target target);

void				lodge_framebuffer_bind(lodge_framebuffer_t framebuffer);
void				lodge_framebuffer_unbind();

void				lodge_framebuffer_clear_color(lodge_framebuffer_t framebuffer, uint32_t index, vec4 clear_value);
void				lodge_framebuffer_clear_depth(lodge_framebuffer_t framebuffer, float depth_value);
void				lodge_framebuffer_clear_stencil(lodge_framebuffer_t framebuffer, int32_t stencil_value);
#if 0
void				lodge_framebuffer_clear_depth_stencil(lodge_framebuffer_t framebuffer, float depth_value, int32_t stencil_value);
#endif

#endif // _FRAMEBUFFER_H
