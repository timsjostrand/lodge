#include "lodge_framebuffer.h"

#include "lodge.h"
#include "lodge_opengl.h"
#include "lodge_texture.h"

#include <stdlib.h>

#define texture_debug(...) debugf("texture", __VA_ARGS__)
#define texture_error(...) errorf("texture", __VA_ARGS__)

#define framebuffer_debug(...) debugf("framebuffer", __VA_ARGS__)
#define framebuffer_error(...) errorf("framebuffer", __VA_ARGS__)

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

lodge_framebuffer_t lodge_framebuffer_make(struct lodge_framebuffer_desc desc)
{
	GLuint framebuffer;
	glCreateFramebuffers(1, &framebuffer);
	GL_OK_OR_GOTO(fail);

	if(desc.colors_count == 0) {
		glNamedFramebufferDrawBuffer(framebuffer, GL_NONE);
	} else {
		for(uint32_t i = 0; i < desc.colors_count; i++) {
			glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0 + i, lodge_texture_to_gl(desc.colors[i]), 0);
		}
	}

	if(desc.depth) {
		glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, lodge_texture_to_gl(desc.depth), 0);
	}

	if(desc.stencil) {
		glNamedFramebufferTexture(framebuffer, GL_STENCIL_ATTACHMENT, lodge_texture_to_gl(desc.stencil), 0);
	}

	GLenum err = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER); 
	if(err != GL_FRAMEBUFFER_COMPLETE) {
		framebuffer_error("Error: %s\n", lodge_framebuffer_status_to_text(err));
		goto fail;
	}

	return lodge_framebuffer_from_gl(framebuffer);

fail:
	ASSERT_FAIL("Failed to make framebuffer");
	return NULL;
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
	lodge_framebuffer_bind(lodge_framebuffer_default());
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

void lodge_framebuffer_copy(lodge_framebuffer_t dst, lodge_framebuffer_t src, struct lodge_framebuffer_rect dst_rect, struct lodge_framebuffer_rect src_rect)
{
	glBlitNamedFramebuffer(
		lodge_framebuffer_to_gl(src),
		lodge_framebuffer_to_gl(dst),
		src_rect.x0, src_rect.y0, src_rect.x1, src_rect.y1,
		dst_rect.x0, dst_rect.y0, dst_rect.x1, dst_rect.y1,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST
	);
}