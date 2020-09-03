#ifndef _LODGE_OPENGL
#define _LODGE_OPENGL

#include "lodge.h"
#include "lodge_texture.h"
#include "lodge_sampler.h"
#include <GL/glew.h>

struct lodge_drawable;
typedef struct lodge_drawable* lodge_drawable_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

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

static GLuint lodge_texture_to_gl(lodge_texture_t tex)
{
	ASSERT(tex > 0);
	return (GLuint)tex;
}

static lodge_texture_t lodge_texture_from_gl(GLuint gl_tex)
{
	return (lodge_texture_t)gl_tex;
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

static GLuint lodge_drawable_to_gl(const lodge_drawable_t drawable)
{
	ASSERT(drawable != NULL);
	return (GLuint)drawable;
}

static lodge_drawable_t lodge_drawable_from_gl(GLuint drawable)
{
	ASSERT(drawable > 0);
	return (lodge_drawable_t)drawable;
}

static lodge_buffer_object_t lodge_buffer_object_from_gl(const GLuint buffer_object)
{
	ASSERT(buffer_object > 0);
	return (lodge_buffer_object_t)buffer_object;
}

static GLuint lodge_buffer_object_to_gl(const lodge_buffer_object_t buffer_object)
{
	ASSERT(buffer_object != NULL);
	return (GLuint)buffer_object;
}

static lodge_framebuffer_t lodge_framebuffer_from_gl(const GLuint framebuffer)
{
	// Framebuffer 0 <=> default fb
	//ASSERT(framebuffer > 0);
	return (lodge_framebuffer_t)(framebuffer + 1);
}

static GLuint lodge_framebuffer_to_gl(const lodge_framebuffer_t framebuffer)
{
	// Framebuffer 0 <=> default fb
	ASSERT(framebuffer != NULL);
	return (GLuint)(framebuffer) - 1;
}

#endif