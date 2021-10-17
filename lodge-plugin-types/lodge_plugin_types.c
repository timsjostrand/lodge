#include "lodge_plugin_types.h"

#include "lodge_type.h"
#include "lodge_serialize_json.h"

struct lodge_ret lodge_plugin_types_new_inplace(void *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	lodge_types_default_register();
	lodge_json_register_func_indices();
	return lodge_success();
}

struct lodge_plugin_desc lodge_plugin_types()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = 4,
		.name = strview_static("types"),
		.new_inplace = &lodge_plugin_types_new_inplace,
		.free_inplace = NULL,
		.update = NULL,
		.render = NULL,
	};
}
