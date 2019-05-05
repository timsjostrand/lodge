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

int graphics_opengl_init(struct graphics *g)
{
	/* OpenGL. */
	//glViewport( 0, 0, view_width, view_height );
	glClearColor(0.33f, 0.33f, 0.33f, 0.0f);

#ifdef DEPTH
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
#else
	glDisable(GL_DEPTH_TEST);
#endif
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	/* Vertex array. */
	glGenVertexArrays(1, &g->vao_rect);
	glBindVertexArray(g->vao_rect);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	/* Vertex buffer. */
	glGenBuffers(1, &g->vbo_rect);
	glBindBuffer(GL_ARRAY_BUFFER, g->vbo_rect);
	glBufferData(GL_ARRAY_BUFFER, VBO_QUAD_LEN * sizeof(float), rect_vertices, GL_STATIC_DRAW);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

	return GRAPHICS_OK;
}

static const char* loc_opengl_debug_type(GLenum type)
{
	switch(type)
	{
	case GL_DEBUG_TYPE_ERROR:
		return "ERROR";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "DEPRECATED_BEHAVIOR";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "UNDEFINED_BEHAVIOR";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "PORTABILITY";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "PERFORMANCE";
	case GL_DEBUG_TYPE_OTHER:
		return "OTHER";
	case GL_DEBUG_TYPE_MARKER:
		return "MARKER";
	case GL_DEBUG_TYPE_PUSH_GROUP:
		return "PUSH_GROUP";
	case GL_DEBUG_TYPE_POP_GROUP:
		return "POP_GROUP";
	default:
		return "UNKNOWN_TYPE";
	}
}

static const char* loc_opengl_debug_severity(GLenum severity)
{
	switch(severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		return "HIGH";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "MEDIUM";
	case GL_DEBUG_SEVERITY_LOW:
		return "LOW";
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "NOTIFICATION";
	default:
		return "UNKNOWN_SEVERITY";
	}
}

static const char* loc_opengl_debug_source(GLenum source)
{
	switch(source)
	{
	case GL_DEBUG_SOURCE_API:
		return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "WINDOW_SYSTEM";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "SHADER_COMPILER";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "THIRD_PARTY";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "APPLICATION";
	case GL_DEBUG_SOURCE_OTHER:
		return "OTHER";
	default:
		return "UNKNOWN_SOURCE";
	}
}

static void GLAPIENTRY loc_opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	printf("OpenGL: %s/%s/%s:\n `%s`\n",
		loc_opengl_debug_source(source),
		loc_opengl_debug_type(type),
		loc_opengl_debug_severity(severity),
		message
	);
}

/**
 * @param g						A graphics struct to fill in.
 * @param view_width			The width of the view, used for ortho().
 * @param view_height			The height of the view, used for ortho().
 */
int graphics_init(struct graphics *g, think_func_t think, render_func_t render, fps_func_t fps_callback)
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

	/* Init GLEW. */
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return GRAPHICS_GLEW_ERROR;
	}

	/* NOTE: Something in glewInit() is causing an 0x0500 OpenGL error, but the
	* Internet says we should ignore this error. */
	while(glGetError() != GL_NO_ERROR) {
		/* Ignore this error. */
	}

	glEnable(GL_DEBUG_OUTPUT);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);
	glDebugMessageCallback((GLDEBUGPROC)&loc_opengl_debug_message_callback, 0);
	GL_OK_OR_RETURN(GRAPHICS_ERROR);

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
