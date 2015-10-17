/**
 * Parsing of command line arguments for core.c.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core_argv.h"
#include "str.h"

static int core_argv_is_arg(const char *arg, const char *name)
{
	if(arg == NULL || name == NULL) {
		return -1;
	}

	/* Allow both styles: -mount, --mount. */
	if(arg[0] == '-') {
		arg++;
	} else {
		return -1;
	}
	if(arg[0] == '-') {
		arg++;
	}

	size_t arg_len = strnlen(arg, CORE_ARGV_NAME_MAX);
	size_t name_len = strnlen(arg, CORE_ARGV_NAME_MAX);

	if(arg_len != name_len) {
		return -1;
	}

	return (strncmp(arg, name, arg_len) == 0) ? 0 : -1;
}

static int core_argv_get_value(int index, char *dst, int argc, char **argv)
{
	/* End of argument vector? */
	if(index+1 >= argc) {
		return -1;
	}
	/* Is next on argv really a value? */
	if(argv[index+1][0] == '-') {
		return -1;
	}
	strncpy(dst, argv[index+1], CORE_ARGV_VALUE_MAX);
	return 0;
}

int core_argv_parse(struct core_argv *dst, int argc, char **argv)
{
	/* TODO: Nice API for defining possible options and what type of value they
	 * have.*/
	for(int i=1; i < argc; i++) {
		/* --mount <PATH> */
		if(core_argv_is_arg(argv[i], "mount") == 0) {
			if(core_argv_get_value(i, dst->mount, argc, argv) != 0) {
				core_argv_error("Usage: --mount <PATH>\n");
				return -1;
			}
			i++;
		}

		/* --windowed */
		if(core_argv_is_arg(argv[i], "windowed") == 0) {
			dst->windowed = 1;
		}
	}

	return 0;
}
