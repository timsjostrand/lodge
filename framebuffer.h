#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

typedef struct framebuffer* framebuffer_t;
typedef struct texture* texture_t;

enum texture_filter
{
	TEXTURE_FILTER_NEAREST = 0,
	TEXTURE_FILTER_LINEAR,
	TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
	TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
	TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
	TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
};

enum texture_wrap
{
	TEXTURE_WRAP_CLAMP = 0,
	TEXTURE_WRAP_REPEAT,
	TEXTURE_WRAP_REPEAT_MIRRORED
};

struct texture_properties
{
	enum texture_filter filter_min;
	enum texture_filter filter_max;
	enum texture_wrap	wrap_s;
	enum texture_wrap	wrap_t;
};

enum framebuffer_target
{
	FRAMEBUFFER_TARGET_COLOR = 0,
	FRAMEBUFFER_TARGET_DEPTH,
	FRAMEBUFFER_TARGET_STENCIL
};

texture_t		texture_create_rgba(int width, int height);
texture_t		texture_create_depth(int width, int height);
void			texture_destroy(texture_t texture);
void			texture_set_properties(texture_t texture, struct texture_properties* properties);

void			texture_bind(texture_t texture, int slot);
void			texture_unbind(int slot);

framebuffer_t	framebuffer_create();
void			framebuffer_destroy(framebuffer_t framebuffer);

void			framebuffer_attach_texture(framebuffer_t framebuffer, texture_t texture, enum framebuffer_target target);

void			framebuffer_bind(framebuffer_t framebuffer);
void			framebuffer_unbind();

#endif // _FRAMEBUFFER_H
