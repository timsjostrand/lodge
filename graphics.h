#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "math4.h"
#include "shader.h"
#include "log.h"

#include "lodge_window.h"

#include <assert.h>

#define graphics_debug(...) debugf("Graphics", __VA_ARGS__)
#define graphics_error(...) errorf("Graphics", __VA_ARGS__)

/**
 * Exhausts OpenGL Error queue and asserts on an error, but continues execution.
 */
#define GL_OK_OR_ASSERT( message ) { \
		GLenum err = GL_NO_ERROR; \
		while((err = glGetError()) != GL_NO_ERROR) { \
			errorf("OpenGL", "Error 0x%04x in %s:%s:%d: %s\n", err, __FILE__, __FUNCTION__, __LINE__, message); \
			assert(0 && message); \
		} \
	}

/**
 * Exhausts OpenGL Error queue and returns when done, optionally with a return value.
 */
#define GL_OK_OR_RETURN(...) { \
		GLenum err = GL_NO_ERROR; \
		while((err = glGetError()) != GL_NO_ERROR) { \
			errorf("OpenGL", "Error 0x%04x in %s:%s:%d\n", err, __FILE__, __FUNCTION__, __LINE__); \
			assert(0); \
			return __VA_ARGS__; \
		} \
	}

#define GRAPHICS_OK					 0
#define GRAPHICS_ERROR				-1
#define GRAPHICS_SHADER_ERROR		-2
#define GRAPHICS_GLEW_ERROR			-4
#define GRAPHICS_IMAGE_LOAD_ERROR	-5
#define GRAPHICS_TEXTURE_LOAD_ERROR -6

#define GRAPHICS_MODE_FULLSCREEN 0
#define GRAPHICS_MODE_BORDERLESS 1
#define GRAPHICS_MODE_WINDOWED 2

/* Number of components in a vertex (x,y,z,u,v). */
#define VBO_VERTEX_LEN			5
/* Number of vertices in a quad. */
#define VBO_QUAD_VERTEX_COUNT	6
/* Number of components in a quad. */
#define VBO_QUAD_LEN			(VBO_QUAD_VERTEX_COUNT * VBO_VERTEX_LEN)

struct frames;

struct core;

typedef void (*fps_func_t)(struct frames *f);

struct frames {
	int			frames;						/* Number of frames drawn since last_frame_report. */
	double		last_frame_report;			/* When frames were last summed up. */
	double		last_frame;					/* When the last frame was drawn. */
	float		frame_time_min;
	float		frame_time_max;
	float		frame_time_sum;
	float		frame_time_avg;
	fps_func_t	callback;					/* A callback thas is called approximately every 1 sec. */
};

struct graphics;

typedef void(*think_func_t)(struct graphics *g, float delta_time);
typedef void(*render_func_t)(struct graphics *g, float delta_time);

struct graphics {
	lodge_window_t	window;						/* The lodge_window handle. */
	think_func_t	think;						/* This function does thinking. */
	render_func_t	render;						/* This function does rendering. */
	struct frames	frames;						/* Frame debug information. */
	float			delta_time_factor;			/* Delta-time is multiplied with this factor. */
	GLuint			vbo_rect;					/* Vertex Buffer Object. */
	GLuint			vao_rect;					/* Vertex Array Object. */
	float			view_width;
	float			view_height;
};

int		graphics_init(struct graphics *g, think_func_t think, render_func_t render, fps_func_t fps_callback);
int		graphics_opengl_init(struct graphics *g);

void	graphics_free(struct graphics *g);
void	graphics_loop();

#endif
