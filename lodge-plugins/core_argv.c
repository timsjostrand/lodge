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
#include "strview.h"

#include "lodge_window.h"
#include "lodge_hash.h"

#define LODGE_ARGV_STR_MAX 512

#if 0
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
#endif

void lodge_argv_new_inplace(struct lodge_argv *dst, int argc, char **argv)
{
	memset(dst, 0, sizeof(*dst));

	ASSERT(argc >= 1);
	dst->path = strview_make(argv[0], strnlen(argv[0], LODGE_ARGV_STR_MAX));

	for(int i = 1; i < argc; i++) {
		const char *arg_str = argv[i];
		size_t arg_str_len = strnlen(arg_str, LODGE_ARGV_STR_MAX);
		enum lodge_arg_type type = LODGE_ARG_TYPE_FLAG;
		{
			/* Allow both styles: -mount, --mount. */
			if(arg_str_len > 0 && arg_str[0] == '-') {
				arg_str++;
				arg_str_len--;
				if(arg_str_len > 1 && arg_str[0] == '-') {
					arg_str++;
					arg_str_len--;
				}
				if(i+1 < argc && argv[i+1][0] != '-') {
					type = LODGE_ARG_TYPE_KEY_VALUE;
				}
			} else {
				type = LODGE_ARG_TYPE_POSITIONAL;
			}
		}

		struct lodge_arg *arg = &dst->elements[dst->count++];

		arg->type = type;
		arg->key = strview_make(arg_str, arg_str_len);
		arg->key_hash = strview_calc_hash(arg->key);

		if(type == LODGE_ARG_TYPE_KEY_VALUE) {
			const char *value_str = argv[i+1];
			const size_t value_str_len = strnlen(value_str, LODGE_ARGV_STR_MAX);
			ASSERT(value_str_len);
			arg->value = strview_make(value_str, value_str_len);

			i++;
		}
	}
}

void lodge_argv_free_inplace(struct lodge_argv *argv)
{
}

const struct lodge_arg* lodge_argv_get_arg_by_hash(const struct lodge_argv *argv, uint32_t key_hash)
{
	ASSERT_OR(argv) { return NULL; }
	for(uint32_t i = 0; i < argv->count; i++) {
		if(argv->elements[i].key_hash == key_hash) {
			return &argv->elements[i];
		}
	}
	return NULL;
}

const struct lodge_arg* lodge_argv_get_arg_by_name(const struct lodge_argv *argv, strview_t key)
{
	return lodge_argv_get_arg_by_hash(argv, strview_calc_hash(key));
}

bool lodge_argv_is_arg(const struct lodge_argv *argv, strview_t key)
{
	return lodge_argv_get_arg_by_name(argv, key);
}

strview_t lodge_argv_get_str(const struct lodge_argv *argv, strview_t key, strview_t default_value)
{
	const struct lodge_arg *arg = lodge_argv_get_arg_by_name(argv, key);

	if(arg && arg->type == LODGE_ARG_TYPE_KEY_VALUE) {
		return arg->value;
	} else {
		return default_value;
	}
}

bool lodge_argv_get_bool(const struct lodge_argv *argv, strview_t key, bool default_value)
{
	const struct lodge_arg *arg = lodge_argv_get_arg_by_name(argv, key);

	if(arg && arg->type == LODGE_ARG_TYPE_KEY_VALUE) {
		ASSERT_NOT_IMPLEMENTED();
		// TODO(TS): parse true/false from ->value
		return default_value;
	} else if (arg && arg->type == LODGE_ARG_TYPE_FLAG) {
		return true;
	} else {
		return default_value;
	}
}
