//
// `lodge_pipeline` attempts to model the graphics pipeline as a single object.
// 
// A pipeline can be created from a desc struct, and then applied
// (`lodge_pipeline_push`) to fully configure the draw state, and then
// withdrawn (`lodge_pipeline_pop`) to return to the previous state.
//
// A local cache compares the state changes before calling into the underlying
// graphics API.
//
// NOTE(TS): The strategy to implementing this in a sane manner is to add
// features as needed, rather than to strive for completeness.
//

#ifndef _LODGE_PIPELINE_H
#define _LODGE_PIPELINE_H

#include <stdbool.h>

#define LODGE_PIPELINES_MAX				256
#define LODGE_PIPELINE_STACK_MAX		256

struct lodge_pipeline;
typedef struct lodge_pipeline* lodge_pipeline_t;

enum lodge_pipeline_compare_func
{
	LODGE_PIPELINE_COMPARE_NEVER,
	LODGE_PIPELINE_COMPARE_LESS,
	LODGE_PIPELINE_COMPARE_EQUAL,
	LODGE_PIPELINE_COMPARE_LEQUAL,
	LODGE_PIPELINE_COMPARE_GREATER,
	LODGE_PIPELINE_COMPARE_NOTEQUAL,
	LODGE_PIPELINE_COMPARE_GEQUAL,
	LODGE_PIPELINE_COMPARE_ALWAYS
};

struct lodge_depth_stencil_state
{
	bool								depth_write;
	enum lodge_pipeline_compare_func	depth_compare_func;
	bool								stencil_write;
};

enum lodge_blend_factor
{
	LODGE_BLEND_FACTOR_ZERO,
	LODGE_BLEND_FACTOR_ONE,
	LODGE_BLEND_FACTOR_SRC_COLOR,
	LODGE_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	LODGE_BLEND_FACTOR_DST_COLOR,
	LODGE_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	LODGE_BLEND_FACTOR_SRC_ALPHA,
	LODGE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	LODGE_BLEND_FACTOR_DST_ALPHA,
	LODGE_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	LODGE_BLEND_FACTOR_CONSTANT_COLOR,
	LODGE_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	LODGE_BLEND_FACTOR_CONSTANT_ALPHA,
	LODGE_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	LODGE_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	LODGE_BLEND_FACTOR_SRC1_COLOR,
	LODGE_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	LODGE_BLEND_FACTOR_SRC1_ALPHA,
	LODGE_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

struct lodge_blend_state
{
	bool								enabled;
	enum lodge_blend_factor				src_factor_rgb;
	enum lodge_blend_factor				dst_factor_rgb;
	enum lodge_blend_factor				src_factor_alpha;
	enum lodge_blend_factor				dst_factor_alpha;
};

enum lodge_rasterizer_cull_mode
{
	LODGE_RASTERIZER_CULL_MODE_NONE,
	LODGE_RASTERIZER_CULL_MODE_FRONT,
	LODGE_RASTERIZER_CULL_MODE_BACK
};

enum lodge_rasterizer_fill_mode
{
	LODGE_RASTERIZER_FILL_MODE_POINT,
	LODGE_RASTERIZER_FILL_MODE_LINE,
	LODGE_RASTERIZER_FILL_MODE_FILL,
};

struct lodge_rasterizer_state
{
	enum lodge_rasterizer_cull_mode		cull_mode;
	enum lodge_rasterizer_fill_mode		fill_mode;
};

struct lodge_pipeline_desc
{
	struct lodge_depth_stencil_state	depth_stencil;
	struct lodge_blend_state			blend;
	struct lodge_rasterizer_state		rasterizer;
};

struct lodge_pipeline_desc				lodge_pipeline_desc_make();

lodge_pipeline_t						lodge_pipeline_make(struct lodge_pipeline_desc desc);
void									lodge_pipeline_reset(lodge_pipeline_t pipeline);

void									lodge_pipeline_push(lodge_pipeline_t pipeline);
void									lodge_pipeline_pop();

#endif