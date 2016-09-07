/**
 * Basic drawing functionality.
 *
 * Authors: Tim Sjöstrand <tim.sjostrand@gmail.com>
 *			Johan Yngman <johan.yngman@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>
#include <GL/glew.h>

#ifdef EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

#include "graphics.h"
#include "math4.h"
#include "texture.h"
#include "color.h"

static const float rect_vertices[] = {
	// Vertex			 // Texcoord
	-0.5f,	0.5f,  0.0f, 0.0f, 0.0f, // Top-left
	-0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Bottom-left
	 0.5f,	0.5f,  0.0f, 1.0f, 0.0f, // Top-right
	 0.5f,	0.5f,  0.0f, 1.0f, 0.0f, // Top-right
	-0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Bottom-left
	 0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // Bottom-right
};

#ifdef EMSCRIPTEN
struct graphics* graphics_global;
#endif

static int graphics_opengl_init(struct graphics *g, int view_width, int view_height)
{
	/* Global transforms. */
	translate(g->translate, 0.0f, 0.0f, 0.0f);
	scale(g->scale, 10.0f, 10.0f, 1);
	rotate_z(g->rotate, 0);
	ortho(g->projection, 0, view_width, view_height, 0, -1.0f, 1.0f);
	transpose_same(g->projection);

	/* OpenGL. */
	// glViewport( 0, 0, view_width, view_height );
	glClearColor(0.33f, 0.33f, 0.33f, 0.0f);

#ifdef DEPTH
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
#else
	glDisable(GL_DEPTH_TEST);
#endif

	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Vertex buffer. */
	glGenBuffers(1, &g->vbo_rect);
	glBindBuffer(GL_ARRAY_BUFFER, g->vbo_rect);
	glBufferData(GL_ARRAY_BUFFER, VBO_QUAD_LEN * sizeof(float),
			rect_vertices, GL_STATIC_DRAW);

	/* Vertex array. */
	glGenVertexArrays(1, &g->vao_rect);
	glBindVertexArray(g->vao_rect);

	GL_OK_OR_RETURN_NONZERO;

	return GRAPHICS_OK;
}

int graphics_libraries_init(struct graphics *g)
{
	/* Init GLEW. */
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		return GRAPHICS_GLEW_ERROR;
	}

	/* NOTE: Something in the init code above is causing an 0x0500 OpenGL error,
	 * and it will linger in the error queue until the application pops it.
	 * Assuming the aforementioned init code does it's error checking properly,
	 * we can safely exhaust the error queue here to avoid ugly debug print
	 * statements later. */
	while(glGetError() != GL_NO_ERROR) {
		/* Ignore this error. */
	}

	return GRAPHICS_OK;
}

/**
 * @param g						A graphics struct to fill in.
 * @param view_width			The width of the view, used for ortho().
 * @param view_height			The height of the view, used for ortho().
 */
int graphics_init(struct graphics *g, think_func_t think, render_func_t render,
		fps_func_t fps_callback, int view_width, int view_height)
{
	int ret = 0;

#ifdef EMSCRIPTEN
	/* HACK: Need to store reference for emscripten. */
	graphics_global = g;
#endif

	/* Set up the graphics struct properly. */
	g->delta_time_factor = 1.0f;
	g->think = think;
	g->render = render;
	g->frames.callback = fps_callback;

	/* Set up GLEW */
	ret = graphics_libraries_init(g);

	if(ret != GRAPHICS_OK) {
		return ret;
	}

	/* Set up OpenGL. */
	ret = graphics_opengl_init(g, view_width, view_height);

	if(ret != GRAPHICS_OK) {
		return ret;
	}

	return GRAPHICS_OK;
}

void graphics_free(struct graphics *g)
{
	/* Free resources. */
	glDeleteVertexArrays(1, &g->vao_rect);
	glDeleteBuffers(1, &g->vbo_rect);
}

static void graphics_frames_register(struct frames *f, float delta_time)
{
	f->frames++;
	f->frame_time_min = fmin(delta_time, f->frame_time_min);
	f->frame_time_max = fmax(delta_time, f->frame_time_max);
	f->frame_time_sum += delta_time;

	if (lodge_window_get_time() - f->last_frame_report >= 1000.0) {
		f->last_frame_report = lodge_window_get_time();
		f->frame_time_avg = f->frame_time_sum / (float) f->frames;
		if(f->callback != NULL) {
			f->callback(f);
		}
		f->frame_time_max = FLT_MIN;
		f->frame_time_min = FLT_MAX;
		f->frame_time_sum = 0;
		f->frames = 0;
	}
}

void graphics_do_frame(struct graphics *g)
{
	double before = lodge_window_get_time();

	/* Delta-time. */
	float delta_time = 0;
	if(g->frames.last_frame != 0) {
		delta_time = (lodge_window_get_time() - g->frames.last_frame) * g->delta_time_factor;
	}
	g->frames.last_frame = lodge_window_get_time();

	/* Game loop. */
	g->think(g, delta_time);
	g->render(g, delta_time);

	lodge_window_update(g->window);

	/* Register that a frame has been drawn. */
	graphics_frames_register(&g->frames, lodge_window_get_time() - before);
}

#ifdef EMSCRIPTEN
void graphics_do_frame_emscripten()
{
	graphics_do_frame(graphics_global);
}
#endif

void graphics_loop(struct graphics *g)
{
	/* Sanity check. */
	if(!g->render || !g->think) {
		graphics_error("g->render() or g->think() not set!\n");
		return;
	}

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(graphics_do_frame_emscripten, 0, 1);
#else
	while (lodge_window_is_open(g->window)) {
		graphics_do_frame(g);
	}
#endif
}
