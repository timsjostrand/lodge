#include "lodge_plugin.h"

#include "membuf.h"

lodge_plugin_func_t		lodge_plugin_registry_data[255];
lodge_plugin_func_t		*lodge_plugin_registry = &lodge_plugin_registry_data[0];
size_t					lodge_plugin_registry_count = 0;

void lodge_plugin_registry_append(lodge_plugin_func_t plugin_func)
{
	membuf_append(
		membuf_wrap(lodge_plugin_registry_data),
		&lodge_plugin_registry_count,
		&plugin_func,
		sizeof(lodge_plugin_func_t)
	);
}