#ifndef _LODGE_POST_PROCESS_H
#define _LODGE_POST_PROCESS_H

#include "math4.h"

#include <stdint.h>

#define LODGE_POST_PROCESS_STEPS_MAX	256

struct lodge_pipeline;
typedef struct lodge_pipeline* lodge_pipeline_t;

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_drawable;
typedef struct lodge_drawable* lodge_drawable_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

struct lodge_scene_render_pass_params;

typedef void					(*lodge_post_process_func_t)(const void *userdata, struct mvp mvp, struct lodge_scene_render_pass_params *render_pass_params);

struct lodge_post_process
{
	lodge_pipeline_t			pipeline;
	lodge_framebuffer_t			framebuffer;
	lodge_framebuffer_t			framebuffer_result;
	lodge_texture_t				color_texture;

	lodge_drawable_t			drawable;
	lodge_buffer_object_t		buffer_object;

	lodge_post_process_func_t	step_funcs[LODGE_POST_PROCESS_STEPS_MAX];
	const void					*step_userdatas[LODGE_POST_PROCESS_STEPS_MAX];
	size_t						steps_count;
};

struct lodge_post_process*		lodge_post_process_new(uint32_t width, uint32_t height, lodge_texture_t depth);

void							lodge_post_process_new_inplace(struct lodge_post_process *post_process, uint32_t width, uint32_t height, lodge_texture_t depth);
void							lodge_post_process_free_inplace(struct lodge_post_process *post_process);

void							lodge_post_process_add_func(struct lodge_post_process *post_process, lodge_post_process_func_t func, const void *userdata);

void							lodge_post_process_render(struct lodge_post_process *post_process, struct lodge_scene_render_pass_params *render_pass_params);

void							lodge_post_process_resize(struct lodge_post_process *post_process, uint32_t width, uint32_t height, lodge_texture_t depth);

#endif