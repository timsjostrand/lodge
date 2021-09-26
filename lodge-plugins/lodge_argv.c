#include "lodge_argv.h"

#include "str.h"
#include "strview.h"

#include "lodge_window.h"
#include "lodge_hash.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LODGE_ARGV_STR_MAX 512

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

uint32_t lodge_argv_get_u32(const struct lodge_argv *argv, strview_t key, uint32_t default_value)
{
	const struct lodge_arg *arg = lodge_argv_get_arg_by_name(argv, key);
	if(arg && arg->type == LODGE_ARG_TYPE_KEY_VALUE) {
		strview_to_u32(arg->value, &default_value);
	} 
	return default_value;
}

uint64_t lodge_argv_get_u64(const struct lodge_argv *argv, strview_t key, uint64_t default_value)
{
	const struct lodge_arg *arg = lodge_argv_get_arg_by_name(argv, key);
	if(arg && arg->type == LODGE_ARG_TYPE_KEY_VALUE) {
		strview_to_u64(arg->value, &default_value);
	} 
	return default_value;
}