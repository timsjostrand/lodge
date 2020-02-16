#ifndef _LODGE_FRAMEBUFFER_H
#define _LODGE_FRAMEBUFFER_H

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

typedef struct framebuffer* framebuffer_t;

enum framebuffer_target
{
	FRAMEBUFFER_TARGET_COLOR = 0,
	FRAMEBUFFER_TARGET_DEPTH,
	FRAMEBUFFER_TARGET_STENCIL
};

framebuffer_t	framebuffer_create();
void			framebuffer_destroy(framebuffer_t framebuffer);

void			framebuffer_attach_texture(framebuffer_t framebuffer, const lodge_texture_t texture, enum framebuffer_target target);

void			framebuffer_bind(framebuffer_t framebuffer);
void			framebuffer_unbind();

#endif // _FRAMEBUFFER_H
