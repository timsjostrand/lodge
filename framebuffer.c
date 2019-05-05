#include "framebuffer.h"
#include "log.h"

#define texture_debug(...) debugf("texture", __VA_ARGS__)
#define texture_error(...) errorf("texture", __VA_ARGS__)

#define framebuffer_debug(...) debugf("framebuffer", __VA_ARGS__)
#define framebuffer_error(...) errorf("framebuffer", __VA_ARGS__)

#include <GL/glew.h>
#include <stdlib.h>

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

static texture_t texture_create(int width, int height, GLint internal_format, GLint format, GLenum type)
{
	struct texture* texture = malloc(sizeof(struct texture));

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, 0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		texture_error("glTexImage2D failed: %d\n", err);
	}

	texture_properties_set_default(texture);

	return texture;
}

texture_t texture_create_rgba(int width, int height)
{
	return texture_create(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
}

texture_t texture_create_depth(int width, int height)
{
	return texture_create(width, height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
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

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		texture_error("glTexParameteri failed: %d\n", err);
	}
}

void texture_bind(texture_t texture, int slot)
{
	GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(GL_TEXTURE_2D, texture->id);
}

void texture_unbind(int slot)
{
	GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(GL_TEXTURE_2D, 0);
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
	
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		framebuffer_error("glGenFramebuffers failed: %d\n", err);
	}
	
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

static const char* framebuffer_status_to_text(GLenum status)
{
	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return "GL_FRAMEBUFFER_COMPLETE";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	case GL_FRAMEBUFFER_UNSUPPORTED:
		return "GL_FRAMEBUFFER_UNSUPPORTED";
	default:
		return "n/a";
	}
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

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		framebuffer_error("glFramebufferTexture2D failed: %d\n", err);
	}

	err = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	if(err != GL_FRAMEBUFFER_COMPLETE) {
		framebuffer_error("Error: %s\n", framebuffer_status_to_text(err));
	}
}
