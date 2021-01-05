#include "lodge_pipeline.h"

#include "membuf.h"
#include "lodge_opengl.h"

#include <stdint.h>

#define LODGE_PIPELINE_STATE_CHANGED(member) \
	(!prev_state || (new_state->member != prev_state->member))

struct lodge_pipeline_impl
{
	struct lodge_pipeline_desc	desc;
};

static struct lodge_pipeline_impl pipelines[LODGE_PIPELINES_MAX];
static struct lodge_pipeline_impl *pipeline_current = NULL;
static size_t pipelines_count = 0;

static lodge_pipeline_t pipeline_stack[LODGE_PIPELINE_STACK_MAX];
static size_t pipeline_stack_count;

static struct lodge_pipeline_impl* lodge_pipeline_to_impl(lodge_pipeline_t pipeline)
{
	const size_t index = (size_t)pipeline;
	ASSERT(index > 0);
	ASSERT(index <= pipelines_count);
	return &pipelines[index - 1];
}

lodge_pipeline_t lodge_pipeline_make(struct lodge_pipeline_desc desc)
{
	membuf_append(
		membuf_wrap(pipelines),
		&pipelines_count,
		&(struct lodge_pipeline_impl) {
			.desc = desc
		},
		sizeof(struct lodge_pipeline_impl)
	);

	return (lodge_pipeline_t)pipelines_count;
}

struct lodge_pipeline_desc lodge_pipeline_desc_make()
{
	return (struct lodge_pipeline_desc) {
		.depth_stencil = {
			.depth_test = true,
			.depth_write = true,
			.depth_compare_func = LODGE_PIPELINE_COMPARE_LESS,
			.stencil_write = true,
		},
		.blend = {
			.enabled = true,
			.src_factor_rgb = GL_ONE,
			.dst_factor_rgb = GL_ZERO,
			.src_factor_alpha = GL_ONE,
			.dst_factor_alpha = GL_ZERO,
			.blend_op_alpha = LODGE_BLEND_OP_ADD,
			.blend_op_rgb = LODGE_BLEND_OP_ADD,
		},
		.rasterizer = {
			.cull_mode = LODGE_RASTERIZER_CULL_MODE_BACK,
			.fill_mode = LODGE_RASTERIZER_FILL_MODE_FILL,
			.face_winding = LODGE_RASTERIZER_FACE_WINDING_CW,
		}
	};
}

void lodge_pipeline_reset(lodge_pipeline_t pipeline)
{
	//
	// TODO(TS): make sure the stack does not contain `pipeline`
	//
	ASSERT_NOT_IMPLEMENTED();
}

static GLenum lodge_pipeline_compare_func_to_gl(enum lodge_pipeline_compare_func compare_func)
{
	switch(compare_func)
	{
	case LODGE_PIPELINE_COMPARE_NEVER:
		return GL_NEVER;
	case LODGE_PIPELINE_COMPARE_LESS:
		return GL_LESS;
	case LODGE_PIPELINE_COMPARE_EQUAL:
		return GL_EQUAL;
	case LODGE_PIPELINE_COMPARE_LEQUAL:
		return GL_LEQUAL;
	case LODGE_PIPELINE_COMPARE_GREATER:
		return GL_GREATER;
	case LODGE_PIPELINE_COMPARE_NOTEQUAL:
		return GL_NOTEQUAL;
	case LODGE_PIPELINE_COMPARE_GEQUAL:
		return GL_GEQUAL;
	case LODGE_PIPELINE_COMPARE_ALWAYS:
		return GL_ALWAYS;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_LESS;
	}
}

static void lodge_depth_stencil_state_bind(struct lodge_depth_stencil_state *new_state, struct lodge_depth_stencil_state *prev_state)
{
	if(LODGE_PIPELINE_STATE_CHANGED(depth_test)) {
		if(new_state->depth_test) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}

	if(LODGE_PIPELINE_STATE_CHANGED(depth_compare_func)) {
		glDepthFunc(lodge_pipeline_compare_func_to_gl(new_state->depth_compare_func));
	}

	if(LODGE_PIPELINE_STATE_CHANGED(depth_write)) {
		glDepthMask(new_state->depth_write ? GL_TRUE : GL_FALSE);
	}

	if(LODGE_PIPELINE_STATE_CHANGED(stencil_write)) {
		glStencilMask(new_state->stencil_write ? GL_TRUE : GL_FALSE);
	}

	GL_OK_OR_ASSERT("Failed to bind depth stencil state");
}

static GLenum lodge_blend_factor_to_gl(enum lodge_blend_factor blend_factor)
{
	switch(blend_factor)
	{
	case LODGE_BLEND_FACTOR_ZERO:
		return GL_ZERO;
	case LODGE_BLEND_FACTOR_ONE:
		return GL_ONE;
	case LODGE_BLEND_FACTOR_SRC_COLOR:
		return GL_SRC_COLOR;
	case LODGE_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		return GL_ONE_MINUS_SRC_COLOR;
	case LODGE_BLEND_FACTOR_DST_COLOR:
		return GL_DST_COLOR;
	case LODGE_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		return GL_ONE_MINUS_DST_COLOR;
	case LODGE_BLEND_FACTOR_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case LODGE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case LODGE_BLEND_FACTOR_DST_ALPHA:
		return GL_DST_ALPHA;
	case LODGE_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
	case LODGE_BLEND_FACTOR_CONSTANT_COLOR:
		return GL_CONSTANT_COLOR;
	case LODGE_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
		return GL_ONE_MINUS_CONSTANT_COLOR;
	case LODGE_BLEND_FACTOR_CONSTANT_ALPHA:
		return GL_CONSTANT_ALPHA;
	case LODGE_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		return GL_ONE_MINUS_CONSTANT_ALPHA;
	case LODGE_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		return GL_SRC_ALPHA_SATURATE;
	case LODGE_BLEND_FACTOR_SRC1_COLOR:
		return GL_SRC1_COLOR;
	case LODGE_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
		return GL_ONE_MINUS_SRC1_COLOR;
	case LODGE_BLEND_FACTOR_SRC1_ALPHA:
		return GL_SRC1_ALPHA;
	case LODGE_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
		return GL_ONE_MINUS_SRC1_ALPHA;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_ONE;
	}
}

static GLenum lodge_blend_op_to_gl(enum lodge_blend_op blend_op)
{
	switch(blend_op)
	{
	case LODGE_BLEND_OP_ADD:
		return GL_FUNC_ADD;
	case LODGE_BLEND_OP_SUBTRACT:
		return GL_FUNC_SUBTRACT;
	case LODGE_BLEND_OP_REVERSE_SUBTRACT:
		return GL_FUNC_REVERSE_SUBTRACT;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_FUNC_ADD;
	}
}

static void lodge_blend_state_bind(struct lodge_blend_state *new_state, struct lodge_blend_state *prev_state)
{
	if(LODGE_PIPELINE_STATE_CHANGED(enabled)) {
		if(new_state->enabled) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
	}

	if(LODGE_PIPELINE_STATE_CHANGED(src_factor_rgb)
		|| LODGE_PIPELINE_STATE_CHANGED(dst_factor_rgb)
		|| LODGE_PIPELINE_STATE_CHANGED(src_factor_alpha)
		|| LODGE_PIPELINE_STATE_CHANGED(dst_factor_alpha)) {
		glBlendFuncSeparate(
			lodge_blend_factor_to_gl(new_state->src_factor_rgb),
			lodge_blend_factor_to_gl(new_state->dst_factor_rgb),
			lodge_blend_factor_to_gl(new_state->src_factor_alpha),
			lodge_blend_factor_to_gl(new_state->dst_factor_alpha)
		);
	}

	if(LODGE_PIPELINE_STATE_CHANGED(blend_op_rgb) || LODGE_PIPELINE_STATE_CHANGED(blend_op_rgb)) {
		glBlendEquationSeparate(
			lodge_blend_op_to_gl(new_state->blend_op_rgb),
			lodge_blend_op_to_gl(new_state->blend_op_alpha)
		);
	}

	GL_OK_OR_ASSERT("Failed to bind blend state");
}

static GLenum lodge_rasterizer_fill_mode_to_gl(enum lodge_rasterizer_fill_mode fill_mode)
{
	switch(fill_mode) 
	{
	case LODGE_RASTERIZER_FILL_MODE_POINT:
		return GL_POINT;
	case LODGE_RASTERIZER_FILL_MODE_LINE:
		return GL_LINE;
	case LODGE_RASTERIZER_FILL_MODE_FILL:
		return GL_FILL;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_FILL;
	}
}

static GLenum lodge_rasterizer_face_winding_gl(enum lodge_rasterizer_face_winding face_winding)
{
	switch(face_winding) 
	{
	case LODGE_RASTERIZER_FACE_WINDING_CW:
		return GL_CW;
	case LODGE_RASTERIZER_FACE_WINDING_CCW:
		return GL_CCW;
	default:
		ASSERT_NOT_IMPLEMENTED();
		return GL_CW;
	}
}

static void lodge_rasterizer_state_bind(struct lodge_rasterizer_state *new_state, struct lodge_rasterizer_state *prev_state)
{
	if(LODGE_PIPELINE_STATE_CHANGED(fill_mode)) {
		glPolygonMode(GL_FRONT_AND_BACK, lodge_rasterizer_fill_mode_to_gl(new_state->fill_mode));
	}

	if(LODGE_PIPELINE_STATE_CHANGED(cull_mode)) {
		if(new_state->cull_mode == LODGE_RASTERIZER_CULL_MODE_NONE) {
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
			glCullFace(new_state->cull_mode == LODGE_RASTERIZER_CULL_MODE_FRONT ? GL_FRONT : GL_BACK);
		}
	}

	if(LODGE_PIPELINE_STATE_CHANGED(face_winding)) {
		glFrontFace(lodge_rasterizer_face_winding_gl(new_state->face_winding));
	}

	GL_OK_OR_ASSERT("Failed to bind rasterizer state");
}

static void lodge_pipeline_bind(lodge_pipeline_t pipeline)
{
	struct lodge_pipeline_impl *impl = lodge_pipeline_to_impl(pipeline);
	ASSERT(impl);

	if(impl == pipeline_current) {
		return;
	}

	lodge_depth_stencil_state_bind(&impl->desc.depth_stencil, pipeline_current ? &pipeline_current->desc.depth_stencil : NULL);
	lodge_blend_state_bind(&impl->desc.blend, pipeline_current ? &pipeline_current->desc.blend : NULL);
	lodge_rasterizer_state_bind(&impl->desc.rasterizer, pipeline_current ? &pipeline_current->desc.rasterizer : NULL);

	// FIXME(TS): apply defaults if this is first pipeline
	if(!pipeline_current) {
		glEnable(GL_SCISSOR_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	pipeline_current = impl;
}

//
// TODO(TS): this should be aware of the current glContext, 1 pipeline stack per context...
//
void lodge_pipeline_push(lodge_pipeline_t pipeline)
{
	membuf_append(membuf_wrap(pipeline_stack), &pipeline_stack_count, &pipeline, sizeof(lodge_pipeline_t));
	lodge_pipeline_bind(pipeline_stack[pipeline_stack_count - 1]);
}

void lodge_pipeline_pop()
{
	if(pipeline_stack_count > 0) {
		pipeline_stack_count--;
		lodge_pipeline_bind(pipeline_stack[pipeline_stack_count - 1]);
	} else {
		ASSERT_FAIL("lodge_pipeline_{push,pop} mismatch");
	}
}