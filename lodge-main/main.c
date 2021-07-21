#include "lodge.h"
#include "lodge_plugin.h"
#include "lodge_plugins.h"
#include "core_argv.h"

int main(int argc, char **argv)
{
	/* Parse command line arguments. */
	struct lodge_argv args = { 0 };
	lodge_argv_new_inplace(&args, argc, argv);

	/* Find plugins */
	struct lodge_plugins *plugins = lodge_plugins_new();
	struct lodge_ret find_ret = lodge_plugins_find(plugins, &args);
	if(!find_ret.success) {
		errorf("Main", "Failed to find plugins: %s", find_ret.message.s);
		return 1;
	}

	struct lodge_ret init_ret = lodge_plugins_init(plugins);
	if(!init_ret.success) {
		errorf("Main", "Failed to init plugins: %s", init_ret.message.s);
		return 1;
	}

	lodge_plugins_run(plugins);
	lodge_plugins_free(plugins);

	return 0;
}
