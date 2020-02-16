#include "lodge_framebuffer.h"

#include "lodge.h"
#include "lodge_opengl.h"
#include "lodge_texture.h"

#include <stdlib.h>

#define texture_debug(...) debugf("texture", __VA_ARGS__)
#define texture_error(...) errorf("texture", __VA_ARGS__)

#define framebuffer_debug(...) debugf("framebuffer", __VA_ARGS__)
#define framebuffer_error(...) errorf("framebuffer", __VA_ARGS__)

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

void framebuffer_attach_texture(framebuffer_t framebuffer, const texture_t texture, enum framebuffer_target target)
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

	glFramebufferTexture2D(GL_FRAMEBUFFER, gl_target, GL_TEXTURE_2D, texture_to_gl(texture), 0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		framebuffer_error("glFramebufferTexture2D failed: %d\n", err);
	}

	err = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	if(err != GL_FRAMEBUFFER_COMPLETE) {
		framebuffer_error("Error: %s\n", framebuffer_status_to_text(err));
	}
}
