#include "framebuffer.h"

#include <GL/glew.h>

//
// Texture
//

struct texture
{
	GLuint id;
};

GLint texture_filter_opengl(enum texture_filter filter)
{
	switch (filter)
	{
	case TEXTURE_FILTER_NEAREST:
		return GL_NEAREST;
	case TEXTURE_FILTER_LINEAR:
		return GL_LINEAR;
	case TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		return GL_NEAREST_MIPMAP_NEAREST;
	case TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		return GL_NEAREST_MIPMAP_LINEAR;
	case TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		return GL_LINEAR_MIPMAP_NEAREST;
	case TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		return GL_LINEAR_MIPMAP_LINEAR;
	}

	return -1;
}

GLint texture_wrap_opengl(enum texture_wrap wrap)
{
	switch (wrap)
	{
	case TEXTURE_WRAP_CLAMP:
		return GL_CLAMP_TO_EDGE;
	case TEXTURE_WRAP_REPEAT:
		return GL_REPEAT;
	case TEXTURE_WRAP_REPEAT_MIRRORED:
		return GL_MIRRORED_REPEAT;
	}

	return -1;
}

void texture_properties_set_default(texture_t texture)
{
	struct texture_properties properties;
	properties.filter_max = TEXTURE_FILTER_NEAREST;
	properties.filter_min = TEXTURE_FILTER_NEAREST;
	properties.wrap_s = TEXTURE_WRAP_CLAMP;
	properties.wrap_t = TEXTURE_WRAP_CLAMP;
	texture_set_properties(texture, &properties);
}

texture_t texture_create_rgba(int width, int height)
{
	struct texture* texture = malloc(sizeof(struct texture));

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	texture_properties_set_default(texture);

	return texture;
}

void texture_destroy(texture_t texture)
{
	glDeleteTextures(1, &texture->id);
	free(texture);
}

void texture_set_properties(texture_t texture, struct texture_properties* properties)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_opengl(properties->wrap_s));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_opengl(properties->wrap_t));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filter_opengl(properties->filter_min));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filter_opengl(properties->filter_max));
}

//
// Framebuffer
//

struct framebuffer
{
	GLuint id;
};

framebuffer_t framebuffer_create()
{
	struct framebuffer* framebuffer = malloc(sizeof(struct framebuffer));

	glGenFramebuffers(1, &framebuffer->id);
	return framebuffer;
}

void framebuffer_destroy(framebuffer_t framebuffer)
{
	glDeleteFramebuffers(1, &framebuffer->id);
	free(framebuffer);
}

void framebuffer_bind(framebuffer_t framebuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
}

void framebuffer_unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_attach_texture(framebuffer_t framebuffer, texture_t texture, enum framebuffer_target target)
{
	GLint gl_target;

	switch (target)
	{
	case FRAMEBUFFER_TARGET_COLOR:
		gl_target = GL_COLOR_ATTACHMENT0;
		break;
	case FRAMEBUFFER_TARGET_DEPTH:
		gl_target = GL_DEPTH_ATTACHMENT;
		break;
	case FRAMEBUFFER_TARGET_STENCIL:
		gl_target = GL_STENCIL_ATTACHMENT;
		break;
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, gl_target, GL_TEXTURE_2D, texture->id, 0);
}
