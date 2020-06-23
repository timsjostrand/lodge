#include "lodge_plugin_env.h"

#include "env.h"

struct lodge_plugin	lodge_plugin_env()
{
	return (struct lodge_plugin) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct env),
		.name = strview_static("env"),
		.init = NULL,
		.free = NULL,
		.update = NULL,
		.render = NULL,
	};
}