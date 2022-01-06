#include "lodge_plugin_stdio_log.h"

#include "lodge_log.h"

#include <stdio.h>

struct lodge_stdio_log
{
	FILE	*output;
	bool	log_level_disabled[LODGE_LOG_LEVEL_MAX];
};

static const char *lodge_level_labels[LODGE_LOG_LEVEL_MAX] ={
	[LODGE_LOG_LEVEL_DEBUG]		= "DEBUG",
	[LODGE_LOG_LEVEL_INFO]		= "INFO ",
	[LODGE_LOG_LEVEL_WARNING]	= "WARN ",
	[LODGE_LOG_LEVEL_ERROR]		= "ERROR",
};

static void lodge_stdio_log_callback(enum lodge_log_level level, strview_t src, strview_t message, struct lodge_stdio_log *plugin)
{
	ASSERT_OR(plugin) { return; }

	if(plugin->log_level_disabled[level]) {
		return;
	}
	
	fprintf(plugin->output, STRVIEW_PRINTF_FMT, 5, lodge_level_labels[level]);
	fprintf(plugin->output, STRVIEW_PRINTF_FMT, 3, " @ ");
	fprintf(plugin->output, STRVIEW_PRINTF_FMT, STRVIEW_PRINTF_ARG(src));
	fprintf(plugin->output, STRVIEW_PRINTF_FMT, 2, ": ");
	fprintf(plugin->output, STRVIEW_PRINTF_FMT, STRVIEW_PRINTF_ARG(message));

	const bool trailing_new_line = (message.length > 0 && message.s[message.length-1] == '\n');
	if(!trailing_new_line) {
		fprintf(plugin->output, "\n");
	}
}

static struct lodge_ret lodge_stdio_log_new_inplace(struct lodge_stdio_log *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args, void **dependencies)
{
	plugin->output = stderr;
	lodge_log_add_func(&lodge_stdio_log_callback, plugin);
	return lodge_success();
}

static void lodge_stdio_log_free_inplace(struct lodge_stdio_log *plugin)
{
	lodge_log_remove_func(&lodge_stdio_log_callback, plugin);
}

LODGE_PLUGIN_IMPL(lodge_plugin_stdio_log)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_stdio_log),
		.name = strview("stdio_log"),
		.new_inplace = &lodge_stdio_log_new_inplace,
		.free_inplace = &lodge_stdio_log_free_inplace,
	};
}