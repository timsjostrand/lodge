#include "lodge_post_process.h"

#include "math4.h"
#include "membuf.h"

#include "lodge_pipeline.h"
#include "lodge_drawable.h"
#include "lodge_buffer_object.h"
#include "lodge_framebuffer.h"
#include "lodge_texture.h"
#include "lodge_plugin_scene_renderer.h" // FIXME(TS): only for render_pass_params

void lodge_post_process_new_inplace(struct lodge_post_process *post_process, uint32_t width, uint32_t height, lodge_texture_t depth)
{
	const float x = 0.0f;
	const float y = 0.0f;

	const float w = 1.0f / 2.0f;
	const float h = 1.0f / 2.0f;

	const float vertex_data[] = {
		/* Top-left */
		x - w,		// x
		y + h,		// y
		0.0f,		// z
		0.0f,		// u
		0.0f,		// v
		/* Bottom-Left */
		x - w,		// x
		y - h,		// y
		0.0f,		// z
		0.0f,		// u
		1.0f,		// v
		/* Top-right */
		x + w,		// x
		y + h,		// y
		0.0f,		// z
		1.0f,		// u
		0.0f,		// v
		/* Top-right */
		x + w,		// x
		y + h,		// y
		0.0f,		// z
		1.0f,		// u
		0.0f,		// v
		/* Bottom-left */
		x - w,		// x
		y - h,		// y
		0.0f,		// z
		0.0f,		// u
		1.0f,		// v
		/* Bottom-right */
		x + w,		// x
		y - h,		// y
		0.0f,		// z
		1.0f,		// u
		1.0f,		// v
	};

	{
		struct lodge_pipeline_desc pipeline_desc = lodge_pipeline_desc_make();
		pipeline_desc.depth_stencil.depth_compare_func = LODGE_PIPELINE_COMPARE_LEQUAL;
		pipeline_desc.blend.src_factor_rgb = LODGE_BLEND_FACTOR_SRC_ALPHA;
		pipeline_desc.blend.dst_factor_rgb = LODGE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline_desc.rasterizer.cull_mode = LODGE_RASTERIZER_CULL_MODE_NONE;

		post_process->pipeline = lodge_pipeline_make(pipeline_desc);
	}

	post_process->color_texture = NULL;
	post_process->framebuffer_result = NULL;
	post_process->framebuffer = NULL;
	lodge_post_process_resize(post_process, width, height, depth);

	post_process->buffer_object = lodge_buffer_object_make_static(vertex_data, sizeof(vertex_data));

	post_process->drawable = lodge_drawable_make((struct lodge_drawable_desc) {
		.attribs_count = 2,
		.attribs = {
			{
				.name = strview_static("pos"),
				.buffer_object = post_process->buffer_object,
				.offset = 0,
				.float_count = 3,
				.stride = 5 * sizeof(float),
				.instanced = 0
			},
			{
				.name = strview_static("uv"),
				.buffer_object = post_process->buffer_object,
				.offset = 3 * sizeof(float),
				.float_count = 2,
				.stride = 5 * sizeof(float),
				.instanced = 0
			},
		}
	});
}

struct lodge_post_process* lodge_post_process_new(uint32_t width, uint32_t height, lodge_texture_t depth)
{
	struct lodge_post_process* post_process = (struct lodge_post_process*)calloc(1, sizeof(struct lodge_post_process));
	lodge_post_process_new_inplace(post_process, width, height, depth);
	return post_process;
}

void lodge_post_process_free_inplace(struct lodge_post_process *post_process)
{
	lodge_buffer_object_reset(post_process->buffer_object);
	lodge_drawable_reset(post_process->drawable);
}

void lodge_post_process_add_func(struct lodge_post_process *post_process, lodge_post_process_func_t func, const void *userdata)
{
	membuf_append(membuf_wrap(post_process->step_funcs), &post_process->steps_count, &func, sizeof(lodge_post_process_func_t));
	membuf_set(membuf_wrap(post_process->step_userdatas), post_process->steps_count - 1, &userdata, sizeof(const void *));
}

void lodge_post_process_render(struct lodge_post_process *post_process, struct lodge_scene_render_pass_params *render_pass_params)
{
	lodge_pipeline_push(post_process->pipeline);
	lodge_framebuffer_bind(post_process->framebuffer);

	for(size_t i = 0; i < post_process->steps_count; i++) {
		struct mvp mvp = {
			.model = mat4_identity(),
			.view = mat4_identity(),
			.projection = mat4_ortho(-0.5f, 0.5f, 0.5f, -0.5f, -1.0f, 1.0f),
		};

		post_process->step_funcs[i](post_process->step_userdatas[i], mvp, render_pass_params);

		lodge_drawable_render_triangles(post_process->drawable, 0, 6);
	}

	lodge_framebuffer_unbind();
	lodge_pipeline_pop(post_process->pipeline);
}

void lodge_post_process_resize(struct lodge_post_process *post_process, uint32_t width, uint32_t height, lodge_texture_t depth)
{
	{
		lodge_texture_reset(post_process->color_texture);
		post_process->color_texture = lodge_texture_2d_make((struct lodge_texture_2d_desc) {
			.width = width,
			.height = height,
			.mipmaps_count = 1,
			.texture_format = LODGE_TEXTURE_FORMAT_RGB16F,
		});
	}

	{
		lodge_framebuffer_reset(post_process->framebuffer_result);
		post_process->framebuffer_result = lodge_framebuffer_make(&(struct lodge_framebuffer_desc) {
			.colors_count = 1,
			.colors = {
				post_process->color_texture
			},
			.depth = depth,
			.stencil = NULL
		});
	}

	{
		lodge_framebuffer_reset(post_process->framebuffer);
		post_process->framebuffer = lodge_framebuffer_make(&(struct lodge_framebuffer_desc) {
			.colors_count = 1,
			.colors = {
				post_process->color_texture
			},
			.depth = NULL,
			.stencil = NULL
		});
	}
}
