#ifndef _LODGE_OPENGL
#define _LODGE_OPENGL

#include "lodge.h"
#include "lodge_texture.h"
#include "lodge_sampler.h"
#include <GL/glew.h>

/**
 * Exhausts the OpenGL Error queue and returns when done (optionally with a return value).
 */
#define GL_OK_OR_RETURN(...) { \
		GLenum err = GL_NO_ERROR; \
		while((err = glGetError()) != GL_NO_ERROR) { \
			errorf("OpenGL", "Error 0x%04x in %s:%s:%d\n", err, __FILE__, __FUNCTION__, __LINE__); \
			ASSERT(0); \
			return __VA_ARGS__; \
		} \
	}

/**
 * Exhausts the OpenGL Error queue and asserts on an error, but continues execution.
 */
#define GL_OK_OR_ASSERT(message) { \
		GLenum err = GL_NO_ERROR; \
		while((err = glGetError()) != GL_NO_ERROR) { \
			errorf("OpenGL", "Error 0x%04x in %s:%s:%d: %s\n", err, __FILE__, __FUNCTION__, __LINE__, (message)); \
			ASSERT_FAIL((message)); \
		} \
	}

#define GL_OK_OR_GOTO(label) { \
		GLenum err = GL_NO_ERROR; \
		while((err = glGetError()) != GL_NO_ERROR) { \
			errorf("OpenGL", "Error 0x%04x in %s:%s:%d\n", err, __FILE__, __FUNCTION__, __LINE__); \
			ASSERT(0); \
			goto label; \
		} \
	}

static GLuint texture_to_gl(texture_t tex)
{
	ASSERT(tex > 0);
	return (GLuint)tex;
}

static texture_t texture_from_gl(GLuint gl_tex)
{
	return (texture_t)gl_tex;
}

static GLuint lodge_sampler_to_gl(lodge_sampler_t sampler)
{
	ASSERT(sampler != NULL);
	return (GLuint)sampler;
}

static lodge_sampler_t lodge_sampler_from_gl(GLuint name)
{
	return (lodge_sampler_t)name;
}

#endif