#include "lodge_framebuffer.h"

#include "lodge.h"
#include "lodge_opengl.h"
#include "lodge_texture.h"

#include <stdlib.h>

#define texture_debug(...) debugf("texture", __VA_ARGS__)
#define texture_error(...) errorf("texture", __VA_ARGS__)

#define framebuffer_debug(...) debugf("framebuffer", __VA_ARGS__)
#define framebuffer_error(...) errorf("framebuffer", __VA_ARGS__)

lodge_framebuffer_t lodge_framebuffer_make()
{
	GLuint framebuffer;

	glGenFramebuffers(1, &framebuffer);
	
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		framebuffer_error("glGenFramebuffers failed: %d\n", err);
	}
	
	return lodge_framebuffer_from_gl(framebuffer);
}

void lodge_framebuffer_reset(lodge_framebuffer_t framebuffer)
{
	glDeleteFramebuffers(1, &(GLuint){ lodge_framebuffer_to_gl(framebuffer) });
}

lodge_framebuffer_t	lodge_framebuffer_default()
{
	return lodge_framebuffer_from_gl(0);
}

void lodge_framebuffer_bind(lodge_framebuffer_t framebuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, lodge_framebuffer_to_gl(framebuffer));
}

void lodge_framebuffer_unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static const char* lodge_framebuffer_status_to_text(GLenum status)
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

void lodge_framebuffer_attach_texture(lodge_framebuffer_t framebuffer, const lodge_texture_t texture, enum framebuffer_target target)
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

	glFramebufferTexture2D(GL_FRAMEBUFFER, gl_target, GL_TEXTURE_2D, lodge_texture_to_gl(texture), 0);

	// TODO(TS): use glDrawBuffer(GL_NONE) if depth only

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		framebuffer_error("glFramebufferTexture2D failed: %d\n", err);
	}

	err = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	if(err != GL_FRAMEBUFFER_COMPLETE) {
		framebuffer_error("Error: %s\n", lodge_framebuffer_status_to_text(err));
	}
}

void lodge_framebuffer_clear_color(lodge_framebuffer_t framebuffer, uint32_t index, vec4 clear_value)
{
	glClearNamedFramebufferfv(lodge_framebuffer_to_gl(framebuffer), GL_COLOR, (GLint)index, clear_value.v);
	GL_OK_OR_ASSERT("Failed to clear framebuffer color");
}

void lodge_framebuffer_clear_depth(lodge_framebuffer_t framebuffer, float depth_value)
{
	// FIXME(TS): must index always be == 0?
	glClearNamedFramebufferfv(lodge_framebuffer_to_gl(framebuffer), GL_DEPTH, (GLint)0, &(GLfloat){ depth_value });
	GL_OK_OR_ASSERT("Failed to clear framebuffer depth");
}

void lodge_framebuffer_clear_stencil(lodge_framebuffer_t framebuffer, int32_t stencil_value)
{
	// FIXME(TS): must index always be == 0?
	glClearNamedFramebufferiv(lodge_framebuffer_to_gl(framebuffer), GL_STENCIL, (GLint)0, &(GLint){ stencil_value });
	GL_OK_OR_ASSERT("Failed to clear framebuffer stencil");
}

#if 0
void lodge_framebuffer_clear_depth_stencil(lodge_framebuffer_t framebuffer, float depth_value, int32_t stencil_value)
{
	glClearNamedFramebufferfi(lodge_framebuffer_to_gl(framebuffer), GL_DEPTH_STENCIL, (GLfloat)depth_value, (GLint)stencil_value);
	GL_OK_OR_ASSERT("Failed to clear framebuffer depth + stencil");
}
#endif
