#include "lodge_plugin.h"

struct lodge_plugin_desc game_plugin()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(void*),
		.name = strview("lodge-editor"),
	};
}
