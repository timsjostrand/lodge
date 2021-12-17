#include "lodge_script_system.h"

#include "lodge_plugin_script.h"
#include "lodge_script_component.h"

#include "lodge_ns_graph.h"
#include "lodge_type_asset.h"

#include "lodge_platform.h"
#include "lodge_time.h"
#include "lodge_type.h"
#include "lodge_scene.h"
#include "lodge_system_type.h"
#include "lodge_script_ctx.h"

//
// For prototype nodes
//
#include "lodge_component_type.h"
#include "lodge_ns_node.h"
#include "lodge_ns_node_type.h"
#include "lodge_editor_controller.h"
#include "lodge_input.h"

#include <stdbool.h>

struct lodge_script_component;

struct lodge_script_system
{
	bool	enabled;
	float	elapsed;
};

static bool lodge_node_editor_controller_state_configure(lodge_node_t node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("key"),
					.type = LODGE_TYPE_I32,
				}
			}
		},
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("down"),
					.type = LODGE_TYPE_BOOL,
				}
			}
		}
	);
	return true;
}

static bool lodge_node_editor_controller_state_run(lodge_node_t node)
{
	struct lodge_script_ctx *ctx = lodge_node_get_graph_context(node);
	if(!ctx) {
		return false;
	}

	//
	// TODO(TS): should do this at configure node type time
	//
	lodge_component_type_t editor_controller_component_type = lodge_component_types_find(strview("editor_controller"));
	if(!editor_controller_component_type) {
		return false;
	}

	struct lodge_editor_controller_component *controller = lodge_scene_get_entity_component(ctx->scene, ctx->entity, editor_controller_component_type);
	if(!controller) {
		return false;
	}

	const int32_t *key = lodge_node_get_i32(node, 0);
	if(!key) {
		return false;
	}

	const bool key_down = lodge_input_is_key_down(controller->input, *key);

	lodge_node_set_boolean(node, 0, key_down);
	return true;
}

#if 0

static bool lodge_node_entity_get_component_configure(lodge_node_t node)
{
	lodge_node_set_pins(node,
		1, (struct lodge_pin[]) {
			{
				.name = strview("type"),
				.type = LODGE_TYPE_COMPONENT_TYPE,
			}
		},
	);
	return true;
}

static bool lodge_node_entity_get_component_run(lodge_node_t node)
{
	struct lodge_script_ctx *ctx = lodge_node_get_graph_context(node);
	if(!ctx) {
		return false;
	}

	const lodge_component_type_t *component_type = lodge_node_get_value_type(node, 0, LODGE_TYPE_COMPONENT_TYPE);
	if(!component_type) {
		return false;
	}

	lodge_node_set_boolean(node, 0, key_down);
	return true;
}

#endif

static void lodge_script_system_new_inplace(struct lodge_script_system *system, lodge_scene_t scene, struct lodge_plugin_script *plugin)
{
	system->enabled = true;

	lodge_node_type_register(strview("editor_controller::key_down"), &lodge_node_editor_controller_state_configure, &lodge_node_editor_controller_state_run);
	//lodge_node_type_register(strview("entity::get_component"), &lodge_node_entity_get_component_configure, &lodge_node_entity_get_component_run);
}

static void lodge_script_system_free_inplace(struct lodge_script_system *system, lodge_scene_t scene, struct lodge_plugin_script *plugin)
{
}

static void lodge_script_system_update(struct lodge_script_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_plugin_script *plugin)
{
	if(!system || !system->enabled) {
		return;
	}

 	struct lodge_script_types types = lodge_plugin_script_get_types(plugin);

	const lodge_timestamp_t before = lodge_timestamp_get();

	lodge_scene_components_foreach(scene, struct lodge_script_component*, it, types.script_component_type) {
		if(!it->enabled) {
			continue;
		}

		lodge_entity_t entity = lodge_scene_get_component_entity(scene, types.script_component_type, it);

		struct lodge_graph *graph = (struct lodge_graph *)lodge_type_get_asset(types.graph_asset_type, it->graph_asset);
		if(graph) {
			struct lodge_script_ctx ctx = {
				.scene = scene,
				.entity = entity,
				.component = it,
			};
			lodge_graph_set_context(graph, &ctx);
			lodge_graph_update(graph);
		}
	}

	const float elapsed = lodge_timestamp_elapsed_us(before);
	system->elapsed = max(system->elapsed * 0.9f + elapsed * 0.1f, 0.0f);
}

lodge_system_type_t lodge_script_system_type_register(struct lodge_plugin_script *plugin)
{
	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview("script_system"),
		.size = sizeof(struct lodge_script_system),
		.new_inplace = &lodge_script_system_new_inplace,
		.free_inplace = &lodge_script_system_free_inplace,
		.update = &lodge_script_system_update,
		.userdata = plugin,
		.properties = {
			.count = 2,
			.elements = {
				{
					.name = strview("enabled"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_script_system, enabled),
					.flags = LODGE_PROPERTY_FLAG_NONE,
				},
				{
					.name = strview("elapsed"),
					.type = LODGE_TYPE_F32,
					.offset = offsetof(struct lodge_script_system, elapsed),
					.flags = LODGE_PROPERTY_FLAG_READ_ONLY,
				},
			}
		}
	});
}
