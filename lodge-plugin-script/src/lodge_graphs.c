#include "lodge_graphs.h"

#include "strview.h"

#include "lodge_platform.h"
#include "lodge_variant.h"
#include "lodge_assets2.h"

#include "lodge_scene.h"
#include "lodge_script_ctx.h"

#include "lodge_ns_graph.h"
#include "lodge_ns_node_type.h"
#include "lodge_ns_node.h"

//
// FIXME(TS): just for debugging
//
#include "lodge_directional_light_component.h"

static bool lodge_node_scene_time_configure(lodge_node_t node)
{
	lodge_node_set_outputs(node, 1, (struct lodge_pin[]) {
		{
			.name = strview("time"),
			.type = LODGE_TYPE_F32,
		}
	});
	return true;
}

static bool lodge_node_scene_time_ms_run(lodge_node_t node)
{
	struct lodge_script_ctx *ctx = lodge_node_get_graph_context(node);
	if(!ctx) {
		return false;
	}
	lodge_node_set_f32(node, 0, lodge_scene_get_time(ctx->scene));
	return true;
}

static bool lodge_node_scene_time_s_run(lodge_node_t node)
{
	struct lodge_script_ctx *ctx = lodge_node_get_graph_context(node);
	if(!ctx) {
		return false;
	}
	lodge_node_set_f32(node, 0, lodge_scene_get_time(ctx->scene) / 1000.0f);
	return true;
}

static bool lodge_node_scene_set_sun_configure(lodge_node_t node)
{
	lodge_node_set_inputs(node, 1, (struct lodge_pin[]) {
		{
			.name = strview("t"),
			.type = LODGE_TYPE_F32,
		}
	});
	return true;
}

static bool lodge_node_scene_set_sun_run(lodge_node_t node)
{
	struct lodge_script_ctx *ctx = lodge_node_get_graph_context(node);
	if(!ctx) {
		return false;
	}

	const float *t = lodge_node_get_f32(node, 0);
	if(!t) {
		return false;
	}

	const vec3 sun_dir = vec3_negate(vec3_make((float)sinf(*t), 0.0f, (float)fabs(cosf(*t))));
	lodge_scene_components_foreach(ctx->scene, struct lodge_directional_light_component*, directional_light, LODGE_COMPONENT_TYPE_DIRECTIONAL_LIGHT) {
		directional_light->dir = sun_dir;
	}

	return true;
}

static bool lodge_assets_graph_new_inplace(struct lodge_assets2 *graphs, strview_t name, lodge_asset_t asset, struct lodge_graph *graph)
{
	lodge_graph_new_inplace(graph, NULL); // FIXME(TS): context...
	return true;
}

static bool lodge_assets_graph_new_default_inplace(struct lodge_assets2 *graphs, struct lodge_graph *graph)
{
	lodge_graph_new_inplace(graph, NULL);

	//lodge_node_type_t type_const_i32 = lodge_node_type_find(strview("const::i32"));
	lodge_node_type_t type_const_f32 = lodge_node_type_find(strview("const::f32"));
	//lodge_node_type_t type_conv_f32_to_i32 = lodge_node_type_find(strview("conv::f32_to_i32"));
	//lodge_node_type_t type_math_add_i32 = lodge_node_type_find(strview("math::add::i32"));
	lodge_node_type_t type_math_mul_f32 = lodge_node_type_find(strview("math::mul::f32"));
	//lodge_node_type_t type_math_sin_f32 = lodge_node_type_find(strview("math::sin::f32"));

	lodge_node_type_t type_scene_time_ms = lodge_node_type_find(strview("scene::time::ms"));
	if(!type_scene_time_ms) {
		lodge_node_type_register(strview("scene::time::ms"), lodge_node_scene_time_configure, lodge_node_scene_time_ms_run);
	}
	lodge_node_type_t type_scene_time_s = lodge_node_type_find(strview("scene::time::s"));
	if(!type_scene_time_s) {
		type_scene_time_s = lodge_node_type_register(strview("scene::time::s"), lodge_node_scene_time_configure, lodge_node_scene_time_s_run);
	}
	lodge_node_type_t type_scene_set_sun = lodge_node_type_find(strview("scene::sun::set"));
	if(!type_scene_set_sun) {
		type_scene_set_sun = lodge_node_type_register(strview("scene::sun::set"), lodge_node_scene_set_sun_configure, lodge_node_scene_set_sun_run);
	}

	lodge_node_t node_time = lodge_graph_add_node(graph, type_scene_time_s);
	lodge_node_t node_rhs = lodge_graph_add_node(graph, type_const_f32);
	lodge_node_t node_mul = lodge_graph_add_node(graph, type_math_mul_f32);
	lodge_node_t node_set_sun = lodge_graph_add_node(graph, type_scene_set_sun);

	struct lodge_variant config = lodge_variant_make_f32(1.5f);
	lodge_node_set_config(node_rhs, &config);

	lodge_node_connect(node_time, 0, node_mul, 0);
	lodge_node_connect(node_rhs, 0, node_mul, 1);
	lodge_node_connect(node_mul, 0, node_set_sun, 0);
	
	lodge_graph_add_main(graph, node_set_sun);

	lodge_graph_configure(graph);

	return true;
}

static void lodge_assets_graphs_free_inplace(struct lodge_assets2 *graphs, strview_t name, lodge_asset_t asset, struct lodge_graph *graph)
{
	lodge_graph_free_inplace(graph);
}

void lodge_graphs_new_inplace(struct lodge_assets2 *graphs)
{
	lodge_assets2_new_inplace(graphs, &(struct lodge_assets2_desc) {
		.name = strview("graphs"),
		.size = lodge_graph_sizeof(),
		.new_inplace = &lodge_assets_graph_new_inplace,
		.new_default_inplace = &lodge_assets_graph_new_default_inplace,
		.reload_inplace = NULL,
		.free_inplace = &lodge_assets_graphs_free_inplace,
	});
}

void lodge_graphs_free_inplace(struct lodge_assets2 *graphs)
{
	lodge_assets2_free_inplace(graphs);
}