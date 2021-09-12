#include "strview.h"

#include "lodge_ns_graph_types.h"
#include "lodge_variant.h"
#include "lodge_ns_graph.h"
#include "lodge_ns_node.h"
#include "lodge_ns_node_type.h"
#include "lodge_assert.h"

#include "lodge_json.h"
#include "lodge_serialize_json.h"

#include "str.h"

char* lodge_graph_to_text(lodge_graph_t graph, size_t *size_out)
{
	lodge_json_t root = lodge_json_new_object();
	lodge_json_t root_version = lodge_json_object_set_number(root, strview_static("version"), 0);
	lodge_json_t root_nodes = lodge_json_object_set_new_array(root, strview_static("nodes"));

	for(size_t node_index = 0, nodes_count = graph->nodes_count; node_index < nodes_count; node_index++) {
		lodge_node_t node = &graph->nodes[node_index];
		strview_t node_type_name = lodge_node_type_get_name(node->type);

		{
			lodge_json_t node_object = lodge_json_array_append_new_object(root_nodes);

			lodge_json_object_set_number(node_object, strview_static("id"), lodge_graph_get_node_id(graph, node));
			lodge_json_object_set_string(node_object, strview_static("type"), node_type_name);

			const struct lodge_variant *config = lodge_node_get_config(node);
			if(lodge_variant_is_set(config)) {
				lodge_json_t config_object = lodge_variant_to_json(config);
				if(!config_object) {
					goto fail;
				}
				lodge_json_object_set(node_object, strview_static("config"), config_object);
			}

			lodge_json_t links_object = lodge_json_object_set_new_array(node_object, strview_static("links"));
			for(lodge_pin_idx_t pin_idx = 0, pin_count = node->inputs.count; pin_idx < pin_count; pin_idx++) {
				const struct lodge_input_pin *pin = &node->inputs.pins[pin_idx];

				if(!pin->connection.node) {
					continue;
				}

				const lodge_node_id_t dst_node_id = lodge_graph_get_node_id(graph, pin->connection.node);

				lodge_json_t link_object = lodge_json_array_append_new_object(links_object);

				lodge_json_object_set_number(link_object, strview_static("src_pin"), pin_idx);
				lodge_json_object_set_number(link_object, strview_static("dst_id"), dst_node_id);
				lodge_json_object_set_number(link_object, strview_static("dst_pin"), pin->connection.pin_index);
			}
		}
	}

	lodge_json_t root_mains = lodge_json_object_set_new_array(root, strview_static("mains"));
	for(size_t main_index = 0, mains_count = graph->mains_count; main_index < mains_count; main_index++ ) {
		lodge_node_t node = graph->mains[main_index];
		lodge_node_id_t node_id = lodge_graph_get_node_id(graph, node);
		lodge_json_array_append_number(root_mains, node_id);
	}

	char* json_text = lodge_json_to_string(root);
	if(!json_text) {
		goto fail;
	}
	*size_out = strlen(json_text) + 1;

	lodge_json_free(root);

	return json_text;

fail:
	ASSERT_FAIL("Failed to serialize graph");
	lodge_json_free(root);
	return NULL;
}

lodge_graph_t lodge_graph_from_text(strview_t data, void *graph_context)
{
#if 0
	lodge_json_t root = lodge_json_from_string(data);
	if(!root) {
		goto fail;
	}

	lodge_graph_t graph = lodge_graph_new(graph_context);
	if(!graph) {
		goto fail;
	}

	double root_version;
	if(!lodge_json_object_get_number(root, strview_static("version"), &root_version)) {
		goto fail;
	}
	ASSERT(root_version == 0.0);

	lodge_json_t root_nodes = lodge_json_object_get_array(root, strview_static("nodes"));
	if(!root_nodes) {
		goto fail;
	}

	lodge_json_t root_mains = lodge_json_object_get_array(root, strview_static("mains"));
	if(!root_mains) {
		goto fail;
	}

	//
	// Create nodes
	//
	{
		lodge_json_t node = NULL;
		cJSON_ArrayForEach(node, root_nodes) {
			cjson_get_or_fail(node_id_object, node, "id");
			size_t node_id = (size_t)node_id_object->valuedouble;
		
			cjson_get_or_fail(node_type_object, node, "type");
			const char *node_type_str = cJSON_GetStringValue(node_type_object);
			if(!node_type_str) {
				goto fail;
			}

			lodge_node_type_t node_type = lodge_node_type_find(strview_make(node_type_str, strlen(node_type_str)));
			if(!node_type) {
				goto fail;
			}

			lodge_node_t node = lodge_graph_add_node(graph, node_type);
			if(!node) {
				goto fail;
			}

			ASSERT(lodge_graph_get_node_index(graph, node) == node_id);
		}
	}

	//
	// TODO(TS): Configure nodes
	//
	for(size_t node_idx = 0, count = lodge_graph_get_node_count(graph); node_idx < count; node_idx++) {

		// TODO(TS): need to configure pins, but not check links right now
		//lodge_node_type_configure_node(  )
	}

	//
	// Connect nodes
	//
	{
		lodge_json_t node = NULL;
		cJSON_ArrayForEach(node, root_nodes) {
			cjson_get_or_fail(node_id_object, node, "id");
			size_t link_src_node_id = (size_t)node_id_object->valuedouble;
		
			cJSON *node_links_object = cJSON_GetObjectItemCaseSensitive(node, "links");
			if(node_links_object) {
				cJSON *link = NULL;
				cJSON_ArrayForEach(link, node_links_object) {
					cjson_get_or_fail(link_src_pin_object, link, "src_pin");
					cjson_get_or_fail(link_dst_node_id_object, link, "dst_id");
					cjson_get_or_fail(link_dst_node_pin_object, link, "dst_pin");

					lodge_pin_idx_t link_src_node_pin = (lodge_pin_idx_t)link_src_pin_object->valuedouble;
					size_t link_dst_node_id = (size_t)link_src_pin_object->valuedouble;
					lodge_pin_idx_t link_dst_node_pin = (lodge_pin_idx_t)link_dst_node_pin_object->valuedouble;

					lodge_node_t src_node = lodge_graph_get_node(graph, link_src_node_id);
					if(!src_node) {
						goto fail;
					}
					lodge_node_t dst_node = lodge_graph_get_node(graph, link_dst_node_id);
					if(!dst_node) {
						goto fail;
					}

					if(!lodge_node_connect(src_node, link_src_node_pin, dst_node, link_dst_node_pin)) {
						goto fail;
					}
				}
			}
		}
	}

	return graph;

fail:
	ASSERT_FAIL("Failed to read graph");
	if(root) {
		lodge_json_free(root);
	}
	if(graph) {
		lodge_graph_free(graph);
	}
	return NULL;
#else
	ASSERT_NOT_IMPLEMENTED();
	return NULL;
#endif
}
