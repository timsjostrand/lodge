#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "math4.h"
#include "shader.h"

#ifdef DEBUG
#define graphics_debug(...) fprintf(stderr, "DEBUG @ Graphics: " __VA_ARGS__)
#else
#define graphics_debug(...) do {} while (0)
#endif

#define graphics_error(...) fprintf(stderr, "ERROR @ Graphics: " __VA_ARGS__)

#define GRAPHICS_OK					 0
#define GRAPHICS_ERROR				-1
#define GRAPHICS_SHADER_ERROR		-2
#define GRAPHICS_GLFW_ERROR			-3
#define GRAPHICS_GLEW_ERROR			-4
#define GRAPHICS_IMAGE_LOAD_ERROR	-5
#define GRAPHICS_TEXTURE_LOAD_ERROR -6

/* Number of components in a vertex (x,y,z,u,v). */
#define VBO_VERTEX_LEN			5
/* Number of vertices in a quad. */
#define VBO_QUAD_VERTEX_COUNT	6
/* Number of components in a quad. */
#define VBO_QUAD_LEN			(VBO_QUAD_VERTEX_COUNT * VBO_VERTEX_LEN)

struct sprite {
	int		type;
	vec4	pos;
	vec4	scale;
	vec4	color;
	float	rotation;
	GLuint	*texture;
};

struct frames;

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

typedef void (*think_func_t)(struct graphics *g, float delta_time);
typedef void (*render_func_t)(struct graphics *g, float delta_time);

struct graphics {
	GLFWwindow		*window;					/* The window handle created by GLFW. */
	think_func_t	think;						/* This function does thinking. */
	render_func_t	render;						/* This function does rendering. */
	struct frames	frames;						/* Frame debug information. */
	float			delta_time_factor;			/* Delta-time is multiplied with this factor. */
	GLuint			vbo_rect;					/* Vertex Buffer Object. */
	GLuint			vao_rect;					/* Vertex Array Object. */
	mat4			projection;					/* Projection matrix. */
	mat4			translate;					/* Global translation matrix. */
	mat4			rotate;						/* Global rotation matrix. */
	mat4			scale;						/* Global scale matrix. */
};

int		graphics_init(struct graphics *g, think_func_t think, render_func_t render,
				fps_func_t fps_callback, int view_width, int view_height, int windowed);
void	graphics_free(struct graphics *g);
void	graphics_loop();

double	now();

void	sprite_render(struct sprite *sprite, struct shader *s, struct graphics *g);

#endif
